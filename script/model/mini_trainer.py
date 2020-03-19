#!/usr/bin/env python3

import glob
import os
import numpy as np
import argparse
import pickle
import logging

from sklearn import model_selection

import io_util
import model
import opunit_data
import data_info
import data_transform
import training_util
import logging_util

from type import Target

np.set_printoptions(precision=4)
np.set_printoptions(edgeitems=10)
np.set_printoptions(suppress=True)


class MiniTrainer:
    """
    Trainer for the mini models
    """

    def __init__(self, input_path, model_metrics_path, ml_models, test_ratio):
        self.input_path = input_path
        self.model_metrics_path = model_metrics_path
        self.ml_models = ml_models
        self.test_ratio = test_ratio

    def train(self):
        """Train the mini-models

        :return: the map of the trained models
        """

        data_list = []

        # First get the data for all mini runners
        for filename in glob.glob(os.path.join(self.input_path, '*.csv')):
            print(filename)
            data_list += opunit_data.get_mini_runner_data(filename)

        model_map = {}
        # train the models for all the operating units
        for data in data_list:
            x_train, x_test, y_train, y_test = model_selection.train_test_split(data.x, data.y,
                                                                                test_size=self.test_ratio,
                                                                                random_state=0)

            # Write the first header rwo to the result file
            metrics_path = "{}/{}.csv".format(self.model_metrics_path, data.opunit.name.lower())
            prediction_path = "{}/{}_prediction.csv".format(self.model_metrics_path, data.opunit.name.lower())
            training_util.create_metrics_and_prediction_files(metrics_path, prediction_path)

            methods = self.ml_models
            # Only use linear regression for the arithmetic operating units
            if data.opunit in data_info.arithmetic_opunits:
                methods = ["lr"]

            # Also test the prediction with the target transformer (if specified for the operating unit)
            transformers = [None]
            modeling_transformer = data_transform.opunit_modeling_trainsformer_map[data.opunit]
            if modeling_transformer is not None:
                transformers.append(modeling_transformer)

            min_percentage_error = 1
            pred_results = None
            elapsed_us_index = data_info.target_csv_index[Target.ELAPSED_US]

            for transformer in transformers:
                for method in methods:
                    # Train the model
                    logging.info("{} {}".format(data.opunit.name, method))
                    regressor = model.Model(method, modeling_transformer=transformer)
                    regressor.train(x_train, y_train)

                    # Evaluate on both the training and test set
                    results = []
                    evaluate_data = [(x_train, y_train), (x_test, y_test)]
                    train_test_label = ["Train", "Test"]
                    for i, d in enumerate(evaluate_data):
                        evaluate_x = d[0]
                        evaluate_y = d[1]

                        y_pred = regressor.predict(evaluate_x)
                        logging.debug("x shape: {}".format(evaluate_x.shape))
                        logging.debug("y shape: {}".format(y_pred.shape))
                        percentage_error = np.average(np.abs(evaluate_y - y_pred) / (evaluate_y + 1), axis=0)
                        results += list(percentage_error) + [""]

                        logging.info('{} Percentage Error: {}'.format(train_test_label[i], percentage_error))

                        # Record the model with the lowest elapsed time prediction (since that might be the most
                        # important prediction)
                        if (i == 1 and percentage_error[elapsed_us_index] < min_percentage_error and transformer ==
                                transformers[-1]):
                            min_percentage_error = percentage_error[elapsed_us_index]
                            model_map[data.opunit] = regressor
                            pred_results = (evaluate_x, y_pred, evaluate_y)

                    # Dump the prediction results
                    transform = " "
                    if transformer is not None:
                        transform = " transform"
                    io_util.write_csv_result(metrics_path, method + transform, results)

                    logging.info("")

                io_util.write_csv_result(metrics_path, "", [])

            # Record the best prediction results on the test data
            training_util.record_predictions(pred_results, prediction_path)

        return model_map


# ==============================================
# main
# ==============================================
if __name__ == '__main__':
    aparser = argparse.ArgumentParser(description='Mini Trainer')
    aparser.add_argument('--input_path', default='mini_runner_input', help='Input file path for the mini runners')
    aparser.add_argument('--model_results_path', default='mini_runner_model_results',
                         help='Prediction results of the mini models')
    aparser.add_argument('--save_path', default='trained_model', help='Path to save the mini models')
    aparser.add_argument('--ml_models', nargs='*', type=str, default=["lr", "rf", "nn"],
                         help='ML models for the mini trainer to evaluate')
    aparser.add_argument('--test_ratio', type=float, default=0.2, help='Test data split ratio')
    aparser.add_argument('--log', default='info', help='The logging level')
    args = aparser.parse_args()

    logging_util.init_logging(args.log)

    trainer = MiniTrainer(args.input_path, args.model_results_path, args.ml_models, args.test_ratio)
    trained_model_map = trainer.train()
    with open(args.save_path + '/mini_model_map.pickle', 'wb') as file:
        pickle.dump(trained_model_map, file)
