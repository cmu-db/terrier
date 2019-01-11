#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "parser/expression/constant_value_expression.h"
#include "parser/expression/function_expression.h"
#include "parser/expression/operator_expression.h"
#include "parser/expression/tuple_value_expression.h"
#include "parser/pg_trigger.h"
#include "parser/postgresparser.h"

#include "util/test_harness.h"

namespace terrier::parser {

class ParserTestBase : public TerrierTest {
 protected:
  /**
   * Initialization
   */
  void SetUp() override {}

  /*
  void InitBasicQueries() {
    // Aggregate
    queries.emplace_back("SELECT * FROM foo;");
    queries.emplace_back("SELECT COUNT(*) FROM foo;");
    queries.emplace_back("SELECT COUNT(DISTINCT id) FROM foo;");
    queries.emplace_back("SELECT MAX(*) FROM foo;");
    queries.emplace_back("SELECT MIN(*) FROM foo;");

    // GROUP BY
    queries.emplace_back("SELECT * FROM foo GROUP BY id,"
                         "name HAVING id > 10;");

    // queries.emplace_back("");
  }
  */

  void TearDown() override {
    // cleanup calls here
  }

  void CheckTable(const std::unique_ptr<TableInfo> &table_info, const std::string &table_name) {
    EXPECT_EQ(table_info->table_name_, table_name);
  }

  PostgresParser pgparser;
};

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DropDBTest) {
  auto stmts = pgparser.BuildParseTree("DROP DATABASE test_db;");
  EXPECT_EQ(stmts.size(), 1);

  auto drop_stmt = reinterpret_cast<DropStatement *>(stmts[0].get());
  EXPECT_EQ(drop_stmt->type_, DropStatement::DropType::kDatabase);
  EXPECT_EQ(drop_stmt->GetDatabaseName(), "test_db");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DropTableTest) {
  auto stmts = pgparser.BuildParseTree("DROP TABLE test_db;");
  EXPECT_EQ(stmts.size(), 1);

  auto drop_stmt = reinterpret_cast<DropStatement *>(stmts[0].get());
  EXPECT_EQ(drop_stmt->type_, DropStatement::DropType::kTable);
  EXPECT_EQ(drop_stmt->GetTableName(), "test_db");
}

/**
 * A basic test for select statements.
 */
// NOLINTNEXTLINE
TEST_F(ParserTestBase, SelectTest) {
  auto stmts = pgparser.BuildParseTree("SELECT * FROM foo;");

  EXPECT_EQ(stmts.size(), 1);
  EXPECT_EQ(stmts[0]->GetType(), StatementType::SELECT);

  auto stmt = std::move(stmts[0]);
  auto select_stmt = reinterpret_cast<SelectStatement *>(stmt.get());
  EXPECT_EQ(select_stmt->from_->table_info_->table_name_, "foo");
  // CheckTable(select_stmt->from_->table_info_, std::string("foo"));
  EXPECT_EQ(select_stmt->select_[0]->GetExpressionType(), ExpressionType::STAR);
}

/**
 * A basic test for TRUNCATE
 */
// NOLINTNEXTLINE
TEST_F(ParserTestBase, TruncateTest) {
  auto stmts = pgparser.BuildParseTree("TRUNCATE TABLE test_db;");
  EXPECT_EQ(stmts.size(), 1);
  EXPECT_EQ(stmts[0]->GetType(), StatementType::DELETE);

  auto delete_stmt = reinterpret_cast<DeleteStatement *>(stmts[0].get());
  EXPECT_EQ(delete_stmt->table_ref_->table_info_->table_name_, "test_db");
  EXPECT_EQ(delete_stmt->expr_, nullptr);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, UpdateTest) {
  auto stmts = pgparser.BuildParseTree("UPDATE students SET grade = 1.0;");

  EXPECT_EQ(stmts.size(), 1);
  EXPECT_EQ(stmts[0]->GetType(), StatementType::UPDATE);

  auto stmt = std::move(stmts[0]);
  auto update_stmt = reinterpret_cast<UpdateStatement *>(stmt.get());
  EXPECT_EQ(update_stmt->table_->table_info_->table_name_, "students");
  // check expression here
  EXPECT_EQ(update_stmt->where_, nullptr);
}

/*
 * All the converted old tests from postgresparser_test.cpp are below.
 * Notable differences:
 * 1. successfully building the parse tree = the statement is valid, no more is_valid checks
 */

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldBasicTest) {
  std::vector<std::string> queries;

  queries.emplace_back("SELECT * FROM foo;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
    EXPECT_EQ(1, stmt_list.size());
    EXPECT_EQ(StatementType::SELECT, stmt_list[0]->GetType());

    // cast stmt_list to derived class pointers
    std::unique_ptr<SelectStatement> statement(dynamic_cast<SelectStatement*>(stmt_list[0].release()));
    EXPECT_EQ("foo", statement->from_->table_info_->table_name_);
    EXPECT_EQ(ExpressionType::STAR, statement->select_[0]->GetExpressionType());
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldAggTest) {
  std::vector<std::string> queries;

  // Select with functional call
  queries.emplace_back("SELECT COUNT(*) FROM foo;");
  queries.emplace_back("SELECT COUNT(DISTINCT id) FROM foo;");
  queries.emplace_back("SELECT MAX(*) FROM foo;");
  queries.emplace_back("SELECT MIN(*) FROM foo;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldGroupByTest) {
  std::vector<std::string> queries;

  // Select with group by clause
  queries.emplace_back("SELECT * FROM foo GROUP BY id, name HAVING id > 10;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldOrderByTest) {
  {
    std::string query = "SELECT * FROM foo ORDER BY id;";
    auto stmt_list = pgparser.BuildParseTree(query);
    auto &sql_stmt = stmt_list[0];
    EXPECT_EQ(sql_stmt->GetType(), StatementType::SELECT);
    auto select_stmt = reinterpret_cast<SelectStatement *>(sql_stmt.get());

    auto &order_by = select_stmt->order_by_;
    EXPECT_NE(order_by, nullptr);

    EXPECT_EQ(order_by->types_.size(), 1);
    EXPECT_EQ(order_by->exprs_.size(), 1);
    EXPECT_EQ(order_by->types_.at(0), OrderType::kOrderAsc);
    auto expr = order_by->exprs_.at(0).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "id");
  }

  {
    std::string query = "SELECT * FROM foo ORDER BY id ASC;";
    auto stmt_list = pgparser.BuildParseTree(query);
    auto &sql_stmt = stmt_list[0];
    EXPECT_EQ(sql_stmt->GetType(), StatementType::SELECT);
    auto select_stmt = reinterpret_cast<SelectStatement *>(sql_stmt.get());
    auto &order_by = select_stmt->order_by_;
    EXPECT_NE(order_by, nullptr);

    EXPECT_EQ(order_by->types_.size(), 1);
    EXPECT_EQ(order_by->exprs_.size(), 1);
    EXPECT_EQ(order_by->types_.at(0), OrderType::kOrderAsc);
    auto expr = order_by->exprs_.at(0).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "id");
  }

  {
    std::string query = "SELECT * FROM foo ORDER BY id DESC;";
    auto stmt_list = pgparser.BuildParseTree(query);
    auto &sql_stmt = stmt_list[0];
    EXPECT_EQ(sql_stmt->GetType(), StatementType::SELECT);
    auto select_stmt = reinterpret_cast<SelectStatement *>(sql_stmt.get());
    auto &order_by = select_stmt->order_by_;
    EXPECT_NE(order_by, nullptr);

    EXPECT_EQ(order_by->types_.size(), 1);
    EXPECT_EQ(order_by->exprs_.size(), 1);
    EXPECT_EQ(order_by->types_.at(0), OrderType::kOrderDesc);
    auto expr = order_by->exprs_.at(0).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "id");
  }

  {
    std::string query = "SELECT * FROM foo ORDER BY id, name;";
    auto stmt_list = pgparser.BuildParseTree(query);
    auto &sql_stmt = stmt_list[0];
    EXPECT_EQ(sql_stmt->GetType(), StatementType::SELECT);
    auto select_stmt = reinterpret_cast<SelectStatement *>(sql_stmt.get());
    auto &order_by = select_stmt->order_by_;
    EXPECT_NE(order_by, nullptr);

    EXPECT_EQ(order_by->types_.size(), 2);
    EXPECT_EQ(order_by->exprs_.size(), 2);
    EXPECT_EQ(order_by->types_.at(0), OrderType::kOrderAsc);
    EXPECT_EQ(order_by->types_.at(1), OrderType::kOrderAsc);
    auto expr = order_by->exprs_.at(0).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "id");
    expr = order_by->exprs_.at(1).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "name");
  }

  {
    std::string query = "SELECT * FROM foo ORDER BY id, name DESC;";
    auto stmt_list = pgparser.BuildParseTree(query);
    auto &sql_stmt = stmt_list[0];
    EXPECT_EQ(sql_stmt->GetType(), StatementType::SELECT);
    auto select_stmt = reinterpret_cast<SelectStatement *>(sql_stmt.get());
    auto &order_by = select_stmt->order_by_;
    EXPECT_NE(order_by, nullptr);

    EXPECT_EQ(order_by->types_.size(), 2);
    EXPECT_EQ(order_by->exprs_.size(), 2);
    EXPECT_EQ(order_by->types_.at(0), OrderType::kOrderAsc);
    EXPECT_EQ(order_by->types_.at(1), OrderType::kOrderDesc);
    auto expr = order_by->exprs_.at(0).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "id");
    expr = order_by->exprs_.at(1).get();
    EXPECT_EQ(expr->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    EXPECT_EQ((reinterpret_cast<TupleValueExpression *>(expr))->GetColumnName(), "name");
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldConstTest) {
  std::vector<std::string> queries;

  // TODO(WAN): disabled until we have string types
  // Select constants
  queries.emplace_back("SELECT 'str', 1 FROM foo;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldJoinTest) {
  std::vector<std::string> queries;

  // Select with join
  queries.emplace_back("SELECT * FROM foo INNER JOIN bar ON foo.id=bar.id AND foo.val > bar.val;");
  queries.emplace_back("SELECT * FROM foo LEFT JOIN bar ON foo.id=bar.id;");
  queries.emplace_back("SELECT * FROM foo RIGHT JOIN bar ON foo.id=bar.id AND foo.val > bar.val;");
  queries.emplace_back("SELECT * FROM foo FULL OUTER JOIN bar ON foo.id=bar.id AND foo.val > bar.val;");
  queries.emplace_back("SELECT * FROM foo JOIN bar ON foo.id=bar.id JOIN baz ON foo.id2=baz.id2;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);

    // TODO(WAN): this was a pretty jank way to test, also won't scale
    // Test for multiple table join
    if (query == "SELECT * FROM foo JOIN bar ON foo.id=bar.id JOIN baz ON foo.id2=baz.id2;") {
      auto select_stmt = reinterpret_cast<SelectStatement *>(stmt_list[0].get());
      auto join_table = select_stmt->from_.get();
      EXPECT_TRUE(join_table->type_ == TableReferenceType::JOIN);
      auto l_join = join_table->join_->left_.get();
      auto r_table = join_table->join_->right_.get();
      EXPECT_TRUE(l_join->type_ == TableReferenceType::JOIN);
      EXPECT_TRUE(r_table->type_ == TableReferenceType::NAME);
    }
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldNestedQueryTest) {
  std::vector<std::string> queries;

  // Select with nested query
  queries.emplace_back("SELECT * FROM (SELECT * FROM foo) as t;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldMultiTableTest) {
  std::vector<std::string> queries;

  // Select from multiple tables
  queries.emplace_back("SELECT foo.name FROM (SELECT * FROM bar) as b, foo, bar WHERE foo.id = b.id;");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldColumnUpdateTest) {
  std::vector<std::string> queries;

  // Select with complicated where, tests both BoolExpr and AExpr
  queries.emplace_back("UPDATE CUSTOMER SET C_BALANCE = C_BALANCE, C_DELIVERY_CNT = C_DELIVERY_CNT WHERE C_W_ID = 2");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);

    EXPECT_EQ(stmt_list.size(), 1);
    auto &sql_stmt = stmt_list[0];

    EXPECT_EQ(sql_stmt->GetType(), StatementType::UPDATE);
    auto update_stmt = reinterpret_cast<UpdateStatement *>(sql_stmt.get());
    auto table = update_stmt->table_.get();
    auto &updates = update_stmt->updates_;
    auto where_clause = update_stmt->where_.get();

    EXPECT_NE(table, nullptr);
    EXPECT_NE(table->table_info_, nullptr);
    EXPECT_EQ(table->table_info_->table_name_, "customer");

    EXPECT_EQ(updates.size(), 2);
    EXPECT_EQ(updates[0]->column_, "c_balance");
    EXPECT_EQ(updates[0]->value_->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    auto column_value_0 = reinterpret_cast<TupleValueExpression *>(updates[0]->value_.get());
    EXPECT_EQ(column_value_0->GetColumnName(), "c_balance");

    EXPECT_EQ(updates[1]->column_, "c_delivery_cnt");
    EXPECT_EQ(updates[1]->value_->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    auto column_value_1 = reinterpret_cast<TupleValueExpression *>(updates[1]->value_.get());
    EXPECT_EQ(column_value_1->GetColumnName(), "c_delivery_cnt");

    EXPECT_NE(where_clause, nullptr);
    EXPECT_EQ(where_clause->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
    auto left_child = where_clause->GetChild(0);
    auto right_child = where_clause->GetChild(1);
    EXPECT_EQ(left_child->GetExpressionType(), ExpressionType::VALUE_TUPLE);
    auto left_tuple = reinterpret_cast<TupleValueExpression *>(left_child.get());
    EXPECT_EQ(left_tuple->GetColumnName(), "c_w_id");

    EXPECT_EQ(right_child->GetExpressionType(), ExpressionType::VALUE_CONSTANT);
    auto right_const = reinterpret_cast<ConstantValueExpression *>(right_child.get());
    EXPECT_EQ(right_const->GetValue().GetType(), type::TypeId::INTEGER);
    EXPECT_EQ(right_const->GetValue().GetIntValue(), 2);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldExpressionUpdateTest) {
  std::string query = "UPDATE STOCK SET S_QUANTITY = 48.0 , S_YTD = S_YTD + 1 WHERE S_I_ID = 68999 AND S_W_ID = 4";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto update_stmt = reinterpret_cast<UpdateStatement *>(stmt_list[0].get());
  EXPECT_EQ(update_stmt->table_->table_info_->table_name_, "stock");

  // Test First Set Condition
  auto &upd0 = update_stmt->updates_.at(0);
  EXPECT_EQ(upd0->column_, "s_quantity");
  auto constant = reinterpret_cast<ConstantValueExpression *>(upd0->value_.get());
  EXPECT_EQ(constant->GetValue().GetType(), type::TypeId::DECIMAL);
  ASSERT_DOUBLE_EQ(constant->GetValue().GetDecimalValue(), 48.0);

  // Test Second Set Condition
  auto &upd1 = update_stmt->updates_.at(1);
  EXPECT_EQ(upd1->column_, "s_ytd");
  auto op_expr = reinterpret_cast<OperatorExpression *>(upd1->value_.get());
  EXPECT_EQ(op_expr->GetExpressionType(), ExpressionType::OPERATOR_PLUS);
  auto child1 = reinterpret_cast<TupleValueExpression *>(op_expr->GetChild(0).get());
  EXPECT_EQ(child1->GetColumnName(), "s_ytd");
  auto child2 = reinterpret_cast<ConstantValueExpression *>(op_expr->GetChild(1).get());
  EXPECT_EQ(child2->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(child2->GetValue().GetIntValue(), 1);

  // Test Where clause
  auto where = reinterpret_cast<OperatorExpression *>(update_stmt->where_.get());
  EXPECT_EQ(where->GetExpressionType(), ExpressionType::CONJUNCTION_AND);

  auto cond1 = reinterpret_cast<OperatorExpression *>(where->GetChild(0).get());
  EXPECT_EQ(cond1->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
  auto column = reinterpret_cast<TupleValueExpression *>(cond1->GetChild(0).get());
  EXPECT_EQ(column->GetColumnName(), "s_i_id");
  constant = reinterpret_cast<ConstantValueExpression *>(cond1->GetChild(1).get());
  EXPECT_EQ(constant->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(constant->GetValue().GetIntValue(), 68999);

  auto cond2 = reinterpret_cast<OperatorExpression *>(where->GetChild(1).get());
  EXPECT_EQ(cond2->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
  column = reinterpret_cast<TupleValueExpression *>(cond2->GetChild(0).get());
  EXPECT_EQ(column->GetColumnName(), "s_w_id");
  constant = reinterpret_cast<ConstantValueExpression *>(cond2->GetChild(1).get());
  EXPECT_EQ(constant->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(constant->GetValue().GetIntValue(), 4);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldStringUpdateTest) {
  // TODO(WAN): need varchar support, see also test bottom
  // Select with complicated where, tests both BoolExpr and AExpr
  std::string query =
      "UPDATE ORDER_LINE SET OL_DELIVERY_D = '2016-11-15 15:07:37' WHERE OL_O_ID = 2101 AND OL_D_ID = 2";

  auto stmt_list = pgparser.BuildParseTree(query);
  auto &sql_stmt = stmt_list[0];

  // Check root type
  EXPECT_EQ(sql_stmt->GetType(), StatementType::UPDATE);
  auto update = reinterpret_cast<UpdateStatement *>(sql_stmt.get());

  // Check table name
  auto &table_ref = update->table_;
  EXPECT_EQ(table_ref->table_info_->table_name_, "order_line");

  // Check where expression
  auto where = update->where_.get();
  EXPECT_EQ(where->GetExpressionType(), ExpressionType::CONJUNCTION_AND);
  EXPECT_EQ(where->GetChildrenSize(), 2);

  auto child0 = where->GetChild(0);
  auto child1 = where->GetChild(1);
  EXPECT_EQ(child0->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
  EXPECT_EQ(child1->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
  EXPECT_EQ(child0->GetChildrenSize(), 2);
  EXPECT_EQ(child1->GetChildrenSize(), 2);

  auto child00 = child0->GetChild(0);
  auto child10 = child1->GetChild(0);
  EXPECT_EQ(child00->GetExpressionType(), ExpressionType::VALUE_TUPLE);
  EXPECT_EQ(child10->GetExpressionType(), ExpressionType::VALUE_TUPLE);
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(child00.get())->GetColumnName(), "ol_o_id");
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(child10.get())->GetColumnName(), "ol_d_id");

  auto child01 = child0->GetChild(1);
  auto child11 = child1->GetChild(1);
  EXPECT_EQ(child01->GetExpressionType(), ExpressionType::VALUE_CONSTANT);
  EXPECT_EQ(child11->GetExpressionType(), ExpressionType::VALUE_CONSTANT);
  EXPECT_EQ(reinterpret_cast<ConstantValueExpression *>(child01.get())->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(reinterpret_cast<ConstantValueExpression *>(child01.get())->GetValue().GetIntValue(), 2101);
  EXPECT_EQ(reinterpret_cast<ConstantValueExpression *>(child11.get())->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(reinterpret_cast<ConstantValueExpression *>(child11.get())->GetValue().GetIntValue(), 2);

  // Check update clause
  auto &update_clause = update->updates_[0];
  EXPECT_EQ(update_clause->column_, "ol_delivery_d");
  auto &value = update_clause->value_;
  EXPECT_EQ(value->GetExpressionType(), ExpressionType::VALUE_CONSTANT);
  // TODO(WAN): need varchar support
  // EXPECT_EQ(((expression::ConstantValueExpression *)value)->GetValue().ToString(), "2016-11-15 15:07:37");
  // EXPECT_EQ(((expression::ConstantValueExpression *)value)->GetValueType(), type::TypeId::VARCHAR);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldDeleteTest) {
  // Simple delete
  std::string query = "DELETE FROM foo;";
  auto stmt_list = pgparser.BuildParseTree(query);

  EXPECT_EQ(stmt_list.size(), 1);
  EXPECT_EQ(stmt_list[0]->GetType(), StatementType::DELETE);
  auto delstmt = reinterpret_cast<DeleteStatement *>(stmt_list[0].get());
  EXPECT_EQ(delstmt->GetTableName(), "foo");
  EXPECT_TRUE(delstmt->expr_ == nullptr);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldDeleteTestWithPredicate) {
  // Delete with a predicate
  std::string query = "DELETE FROM foo WHERE id=3;";
  auto stmt_list = pgparser.BuildParseTree(query);

  EXPECT_EQ(stmt_list.size(), 1);
  EXPECT_EQ(stmt_list[0]->GetType(), StatementType::DELETE);
  auto delstmt = reinterpret_cast<DeleteStatement *>(stmt_list[0].get());
  EXPECT_EQ(delstmt->GetTableName(), "foo");
  EXPECT_TRUE(delstmt->expr_ != nullptr);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldInsertTest) {
  // TODO(WAN): need NULL value support
  // Insert multiple tuples into the table
  std::string query = "INSERT INTO foo VALUES (NULL, 2, 3), (4, 5, 6);";
  auto stmt_list = pgparser.BuildParseTree(query);

  EXPECT_EQ(1, stmt_list.size());
  EXPECT_TRUE(stmt_list[0]->GetType() == StatementType::INSERT);
  auto insert_stmt = reinterpret_cast<InsertStatement *>(stmt_list[0].get());
  EXPECT_EQ("foo", insert_stmt->GetTableName());
  EXPECT_EQ(2, insert_stmt->insert_values_->size());

  // Test NULL Value parsing
  auto constant = reinterpret_cast<ConstantValueExpression *>(insert_stmt->insert_values_->at(0).at(0).get());
  // EXPECT_TRUE(((expression::ConstantValueExpression
  // *)insert_stmt->insert_values.at(0).at(0).get())->GetValue().IsNull()); Test normal value
  constant = reinterpret_cast<ConstantValueExpression *>(insert_stmt->insert_values_->at(1).at(1).get());
  EXPECT_EQ(constant->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(constant->GetValue().GetIntValue(), 5);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCreateTest) {
  std::string query =
      "CREATE TABLE Persons ("
      "id INT NOT NULL UNIQUE, "
      "age INT PRIMARY KEY, "
      "name VARCHAR(255), "
      "c_id INT,"
      "PRIMARY KEY (id),"
      "FOREIGN KEY (c_id) REFERENCES country (cid));";

  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check column definition
  EXPECT_EQ(create_stmt->columns_.size(), 4);
  // Check First column
  auto column = create_stmt->columns_[0].get();
  EXPECT_TRUE(column->is_not_null_);
  EXPECT_TRUE(column->is_unique_);
  EXPECT_TRUE(column->is_primary_);
  EXPECT_EQ(column->name_, "id");
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::INT);
  // Check Second column
  column = create_stmt->columns_[1].get();
  EXPECT_FALSE(column->is_not_null_);
  EXPECT_TRUE(column->is_primary_);
  // Check Third column
  column = create_stmt->columns_[2].get();
  EXPECT_FALSE(column->is_primary_);
  EXPECT_EQ(column->varlen_, 255);

  // Check Foreign Key Constraint
  column = create_stmt->foreign_keys_[0].get();
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::FOREIGN);
  EXPECT_EQ(column->fk_sources_[0], "c_id");
  EXPECT_EQ(column->fk_sinks_[0], "cid");
  EXPECT_EQ(column->fk_sink_table_name_, "country");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldTransactionTest) {
  std::string query = "BEGIN TRANSACTION;";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto transac_stmt = reinterpret_cast<TransactionStatement *>(stmt_list[0].get());
  EXPECT_EQ(transac_stmt->type_, TransactionStatement::kBegin);

  query = "BEGIN;";
  stmt_list = pgparser.BuildParseTree(query);
  transac_stmt = reinterpret_cast<TransactionStatement *>(stmt_list[0].get());
  EXPECT_EQ(transac_stmt->type_, TransactionStatement::kBegin);

  query = "COMMIT TRANSACTION;";
  stmt_list = pgparser.BuildParseTree(query);
  transac_stmt = reinterpret_cast<TransactionStatement *>(stmt_list[0].get());
  EXPECT_EQ(transac_stmt->type_, TransactionStatement::kCommit);

  query = "ROLLBACK;";
  stmt_list = pgparser.BuildParseTree(query);
  transac_stmt = reinterpret_cast<TransactionStatement *>(stmt_list[0].get());
  EXPECT_EQ(transac_stmt->type_, TransactionStatement::kRollback);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCreateIndexTest) {
  std::string query = "CREATE UNIQUE INDEX IDX_ORDER ON oorder (O_W_ID, O_D_ID);";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check attributes
  EXPECT_EQ(create_stmt->create_type_, CreateStatement::kIndex);
  EXPECT_TRUE(create_stmt->unique_index_);
  EXPECT_EQ(create_stmt->index_name_, "idx_order");
  EXPECT_EQ(create_stmt->GetTableName(), "oorder");
  EXPECT_EQ(create_stmt->index_attrs_[0], "o_w_id");
  EXPECT_EQ(create_stmt->index_attrs_[1], "o_d_id");

  query = "CREATE INDEX ii ON t USING SKIPLIST (col);";
  stmt_list = pgparser.BuildParseTree(query);
  create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check attributes
  EXPECT_EQ(create_stmt->create_type_, CreateStatement::kIndex);
  EXPECT_EQ(create_stmt->index_type_, IndexType::SKIPLIST);
  EXPECT_EQ(create_stmt->index_name_, "ii");
  EXPECT_EQ(create_stmt->GetTableName(), "t");

  query = "CREATE INDEX ii ON t (col);";
  stmt_list = pgparser.BuildParseTree(query);
  create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check attributes
  EXPECT_EQ(create_stmt->create_type_, CreateStatement::kIndex);
  EXPECT_EQ(create_stmt->index_type_, IndexType::BWTREE);
  EXPECT_EQ(create_stmt->index_name_, "ii");
  EXPECT_EQ(create_stmt->GetTableName(), "t");

  query = "CREATE INDEX ii ON t USING GIN (col);";

  EXPECT_THROW(pgparser.BuildParseTree(query), NotImplementedException);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldInsertIntoSelectTest) {
  // insert into a table with select sub-query
  std::string query = "INSERT INTO foo select * from bar where id = 5;";
  auto stmt_list = pgparser.BuildParseTree(query);

  EXPECT_EQ(stmt_list.size(), 1);
  EXPECT_TRUE(stmt_list[0]->GetType() == StatementType::INSERT);
  auto insert_stmt = reinterpret_cast<InsertStatement *>(stmt_list[0].get());
  EXPECT_EQ(insert_stmt->GetTableName(), "foo");
  EXPECT_EQ(insert_stmt->insert_values_, nullptr);
  EXPECT_EQ(insert_stmt->select_->GetType(), StatementType::SELECT);
  EXPECT_EQ(insert_stmt->select_->from_->GetTableName(), "bar");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCreateDbTest) {
  std::string query = "CREATE DATABASE tt";
  auto stmt_list = pgparser.BuildParseTree(query);

  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());
  EXPECT_EQ(create_stmt->create_type_, CreateStatement::CreateType::kDatabase);
  EXPECT_EQ(create_stmt->GetDatabaseName(), "tt");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCreateSchemaTest) {
  std::string query = "CREATE SCHEMA tt";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());
  EXPECT_EQ("tt", create_stmt->GetSchemaName());

  // Test default schema name
  query = "CREATE SCHEMA AUTHORIZATION joe";
  stmt_list = pgparser.BuildParseTree(query);
  create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());
  EXPECT_EQ("joe", create_stmt->GetSchemaName());
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldCreateViewTest) {
  // TODO(WAN): need strings
  std::string query = "CREATE VIEW comedies AS SELECT * FROM films WHERE kind = 'Comedy';";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check attributes
  EXPECT_EQ(create_stmt->view_name_, "comedies");
  EXPECT_NE(create_stmt->view_query_, nullptr);
  auto view_query = create_stmt->view_query_.get();
  EXPECT_EQ(view_query->from_->GetTableName(), "films");
  EXPECT_EQ(view_query->select_.size(), 1);
  EXPECT_NE(view_query->where_, nullptr);
  EXPECT_EQ(view_query->where_->GetExpressionType(), ExpressionType::COMPARE_EQUAL);
  EXPECT_EQ(view_query->where_->GetChildrenSize(), 2);
  auto left_child = view_query->where_->GetChild(0);
  EXPECT_EQ(left_child->GetExpressionType(), ExpressionType::VALUE_TUPLE);
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(left_child.get())->GetColumnName(), "kind");
  auto right_child = view_query->where_->GetChild(1);
  EXPECT_EQ(right_child->GetExpressionType(), ExpressionType::VALUE_CONSTANT);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldDistinctFromTest) {
  std::string query = "SELECT id, value FROM foo WHERE id IS DISTINCT FROM value;";
  auto stmt_list = pgparser.BuildParseTree(query);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldConstraintTest) {
  std::string query =
      "CREATE TABLE table1 ("
      "a int DEFAULT 1+2,"
      "b int DEFAULT 1 REFERENCES table2 (bb) ON UPDATE CASCADE,"
      "c varchar(32) REFERENCES table3 (cc) MATCH FULL ON DELETE SET NULL,"
      "d int CHECK (d+1 > 0),"
      "FOREIGN KEY (d) REFERENCES table4 (dd) MATCH SIMPLE ON UPDATE SET "
      "DEFAULT"
      ");";

  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  // Check column definition
  EXPECT_EQ(create_stmt->columns_.size(), 4);

  // Check First column
  auto column = create_stmt->columns_[0].get();
  EXPECT_EQ(column->name_, "a");
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::INT);
  EXPECT_NE(column->default_expr_, nullptr);
  auto default_expr = reinterpret_cast<OperatorExpression *>(column->default_expr_.get());
  EXPECT_NE(default_expr, nullptr);
  EXPECT_EQ(default_expr->GetExpressionType(), ExpressionType::OPERATOR_PLUS);
  EXPECT_EQ(default_expr->GetChildrenSize(), 2);

  auto child0 = reinterpret_cast<ConstantValueExpression *>(default_expr->GetChild(0).get());
  EXPECT_NE(child0, nullptr);
  EXPECT_EQ(child0->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(child0->GetValue().GetIntValue(), 1);

  auto child1 = reinterpret_cast<ConstantValueExpression *>(default_expr->GetChild(1).get());
  EXPECT_NE(child1, nullptr);
  EXPECT_EQ(child1->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(child1->GetValue().GetIntValue(), 2);

  // Check Second column
  column = create_stmt->columns_[1].get();
  EXPECT_EQ(column->name_, "b");
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::INT);

  // Check Third column
  column = create_stmt->columns_[2].get();
  EXPECT_EQ(column->name_, "c");
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::VARCHAR);

  // Check Fourth column
  column = create_stmt->columns_[3].get();
  EXPECT_EQ(column->name_, "d");
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::INT);
  EXPECT_NE(column->check_expr_, nullptr);
  EXPECT_EQ(column->check_expr_->GetExpressionType(), ExpressionType::COMPARE_GREATER_THAN);
  EXPECT_EQ(column->check_expr_->GetChildrenSize(), 2);

  auto check_child1 = reinterpret_cast<OperatorExpression *>(column->check_expr_->GetChild(0).get());
  EXPECT_NE(check_child1, nullptr);
  EXPECT_EQ(check_child1->GetExpressionType(), ExpressionType::OPERATOR_PLUS);
  EXPECT_EQ(check_child1->GetChildrenSize(), 2);
  auto plus_child1 = reinterpret_cast<TupleValueExpression *>(check_child1->GetChild(0).get());
  EXPECT_NE(plus_child1, nullptr);
  EXPECT_EQ(plus_child1->GetColumnName(), "d");
  auto plus_child2 = reinterpret_cast<ConstantValueExpression *>(check_child1->GetChild(1).get());
  EXPECT_NE(plus_child2, nullptr);
  EXPECT_EQ(plus_child2->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(plus_child2->GetValue().GetIntValue(), 1);

  auto check_child2 = reinterpret_cast<ConstantValueExpression *>(column->check_expr_->GetChild(1).get());
  EXPECT_NE(check_child2, nullptr);
  EXPECT_EQ(check_child2->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(check_child2->GetValue().GetIntValue(), 0);

  // Check the foreign key constraint
  column = create_stmt->foreign_keys_[0].get();
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::FOREIGN);
  EXPECT_EQ(column->fk_sinks_.size(), 1);
  EXPECT_EQ(column->fk_sinks_[0], "bb");
  EXPECT_EQ(column->fk_sink_table_name_, "table2");
  EXPECT_EQ(column->fk_update_action_, FKConstrActionType::CASCADE);
  EXPECT_EQ(column->fk_delete_action_, FKConstrActionType::NOACTION);
  EXPECT_EQ(column->fk_match_type_, FKConstrMatchType::SIMPLE);

  column = create_stmt->foreign_keys_[1].get();
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::FOREIGN);
  EXPECT_EQ(column->fk_sinks_.size(), 1);
  EXPECT_EQ(column->fk_sinks_[0], "cc");
  EXPECT_EQ(column->fk_sink_table_name_, "table3");
  EXPECT_EQ(column->fk_update_action_, FKConstrActionType::NOACTION);
  EXPECT_EQ(column->fk_delete_action_, FKConstrActionType::SETNULL);
  EXPECT_EQ(column->fk_match_type_, FKConstrMatchType::FULL);

  column = create_stmt->foreign_keys_[2].get();
  EXPECT_EQ(column->type_, ColumnDefinition::DataType::FOREIGN);
  EXPECT_EQ(column->fk_sources_.size(), 1);
  EXPECT_EQ(column->fk_sources_[0], "d");
  EXPECT_EQ(column->fk_sinks_.size(), 1);
  EXPECT_EQ(column->fk_sinks_[0], "dd");
  EXPECT_EQ(column->fk_sink_table_name_, "table4");
  EXPECT_EQ(column->fk_update_action_, FKConstrActionType::SETDEFAULT);
  EXPECT_EQ(column->fk_delete_action_, FKConstrActionType::NOACTION);
  EXPECT_EQ(column->fk_match_type_, FKConstrMatchType::SIMPLE);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldDataTypeTest) {
  std::string query =
      "CREATE TABLE table1 ("
      "a text,"
      "b varchar(1024),"
      "c varbinary(32)"
      ");";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto create_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  EXPECT_EQ(create_stmt->columns_.size(), 3);

  // Check First column
  auto column = create_stmt->columns_[0].get();
  EXPECT_EQ(column->name_, "a");
  EXPECT_EQ(column->GetValueType(column->type_), type::TypeId::VARCHAR);
  // TODO(WAN): we got an equivalent of this?
  // EXPECT_EQ(peloton::type::PELOTON_TEXT_MAX_LEN, column->varlen);

  // Check Second column
  column = create_stmt->columns_[1].get();
  EXPECT_EQ(column->name_, "b");
  EXPECT_EQ(column->GetValueType(column->type_), type::TypeId::VARCHAR);
  EXPECT_EQ(column->varlen_, 1024);

  // Check Third column
  column = create_stmt->columns_[2].get();
  EXPECT_EQ(column->name_, "c");
  EXPECT_EQ(column->GetValueType(column->type_), type::TypeId::VARBINARY);
  EXPECT_EQ(column->varlen_, 32);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCreateTriggerTest) {
  std::string query =
      "CREATE TRIGGER check_update "
      "BEFORE UPDATE OF balance ON accounts "
      "FOR EACH ROW "
      "WHEN (OLD.balance <> NEW.balance) "
      "EXECUTE PROCEDURE check_account_update();";
  auto stmt_list = pgparser.BuildParseTree(query);

  EXPECT_EQ(stmt_list[0]->GetType(), StatementType::CREATE);
  auto create_trigger_stmt = reinterpret_cast<CreateStatement *>(stmt_list[0].get());

  EXPECT_EQ(create_trigger_stmt->create_type_, CreateStatement::CreateType::kTrigger);
  EXPECT_EQ(create_trigger_stmt->trigger_name_, "check_update");
  EXPECT_EQ(create_trigger_stmt->GetTableName(), "accounts");

  auto funcname = create_trigger_stmt->trigger_funcnames_;
  EXPECT_EQ(funcname.size(), 1);
  EXPECT_EQ(funcname[0], "check_account_update");

  EXPECT_EQ(create_trigger_stmt->trigger_args_.size(), 0);

  auto columns = create_trigger_stmt->trigger_columns_;
  EXPECT_EQ(columns.size(), 1);
  EXPECT_EQ(columns[0], "balance");

  auto &when = create_trigger_stmt->trigger_when_;
  EXPECT_NE(when, nullptr);
  EXPECT_EQ(when->GetExpressionType(), ExpressionType::COMPARE_NOT_EQUAL);
  EXPECT_EQ(when->GetChildrenSize(), 2);

  auto left = when->GetChild(0).get();
  auto right = when->GetChild(1).get();
  EXPECT_EQ(left->GetExpressionType(), ExpressionType::VALUE_TUPLE);
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(left)->GetTableName(), "old");
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(left)->GetColumnName(), "balance");
  EXPECT_EQ(right->GetExpressionType(), ExpressionType::VALUE_TUPLE);
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(right)->GetTableName(), "new");
  EXPECT_EQ(reinterpret_cast<TupleValueExpression *>(right)->GetColumnName(), "balance");

  EXPECT_TRUE(TRIGGER_FOR_ROW(create_trigger_stmt->trigger_type_));

  EXPECT_TRUE(TRIGGER_FOR_BEFORE(create_trigger_stmt->trigger_type_));
  EXPECT_FALSE(TRIGGER_FOR_AFTER(create_trigger_stmt->trigger_type_));
  EXPECT_FALSE(TRIGGER_FOR_INSTEAD(create_trigger_stmt->trigger_type_));

  EXPECT_TRUE(TRIGGER_FOR_UPDATE(create_trigger_stmt->trigger_type_));
  EXPECT_FALSE(TRIGGER_FOR_INSERT(create_trigger_stmt->trigger_type_));
  EXPECT_FALSE(TRIGGER_FOR_DELETE(create_trigger_stmt->trigger_type_));
  EXPECT_FALSE(TRIGGER_FOR_TRUNCATE(create_trigger_stmt->trigger_type_));
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldDropTriggerTest) {
  std::string query = "DROP TRIGGER if_dist_exists ON terrier.films;";
  auto stmt_list = pgparser.BuildParseTree(query);
  EXPECT_EQ(stmt_list[0]->GetType(), StatementType::DROP);
  auto drop_trigger_stmt = reinterpret_cast<DropStatement *>(stmt_list[0].get());

  EXPECT_EQ(drop_trigger_stmt->type_, DropStatement::DropType::kTrigger);
  EXPECT_EQ(drop_trigger_stmt->trigger_name_, "if_dist_exists");
  EXPECT_EQ(drop_trigger_stmt->GetTableName(), "films");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldFuncCallTest) {
  std::string query = "SELECT add(1,a), chr(99) FROM TEST WHERE FUN(b) > 2";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto select_stmt = reinterpret_cast<SelectStatement *>(stmt_list[0].get());

  // Check ADD(1,a)
  auto fun_expr = reinterpret_cast<FunctionExpression *>(select_stmt->select_[0].get());
  EXPECT_NE(fun_expr, nullptr);
  EXPECT_EQ(fun_expr->GetFuncName(), "add");
  EXPECT_EQ(fun_expr->GetChildrenSize(), 2);

  auto const_expr = reinterpret_cast<ConstantValueExpression *>(fun_expr->GetChild(0).get());
  EXPECT_NE(const_expr, nullptr);
  EXPECT_EQ(const_expr->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(const_expr->GetValue().GetIntValue(), 1);

  auto tv_expr = reinterpret_cast<TupleValueExpression *>(fun_expr->GetChild(1).get());
  EXPECT_NE(tv_expr, nullptr);
  EXPECT_EQ(tv_expr->GetColumnName(), "a");

  // Check chr(99)
  fun_expr = reinterpret_cast<FunctionExpression *>(select_stmt->select_[1].get());
  EXPECT_NE(fun_expr, nullptr);
  EXPECT_EQ(fun_expr->GetFuncName(), "chr");
  EXPECT_EQ(fun_expr->GetChildrenSize(), 1);

  // Check FUN(b) > 2
  auto op_expr = reinterpret_cast<OperatorExpression *>(select_stmt->where_.get());
  EXPECT_NE(op_expr, nullptr);
  EXPECT_EQ(op_expr->GetExpressionType(), ExpressionType::COMPARE_GREATER_THAN);

  fun_expr = reinterpret_cast<FunctionExpression *>(op_expr->GetChild(0).get());
  EXPECT_NE(fun_expr, nullptr);
  EXPECT_EQ(fun_expr->GetFuncName(), "fun");
  EXPECT_EQ(fun_expr->GetChildrenSize(), 1);
  tv_expr = reinterpret_cast<TupleValueExpression *>(fun_expr->GetChild(0).get());
  EXPECT_NE(tv_expr, nullptr);
  EXPECT_EQ(tv_expr->GetColumnName(), "b");

  const_expr = reinterpret_cast<ConstantValueExpression *>(op_expr->GetChild(1).get());
  EXPECT_NE(const_expr, nullptr);
  EXPECT_EQ(const_expr->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(const_expr->GetValue().GetIntValue(), 2);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldUDFFuncCallTest) {
  std::string query = "SELECT increment(1,b) FROM TEST;";
  auto stmt_list = pgparser.BuildParseTree(query);
  auto select_stmt = reinterpret_cast<SelectStatement *>(stmt_list[0].get());

  auto fun_expr = reinterpret_cast<FunctionExpression *>(select_stmt->select_[0].get());
  EXPECT_NE(fun_expr, nullptr);
  EXPECT_EQ(fun_expr->GetFuncName(), "increment");
  EXPECT_EQ(fun_expr->GetChildrenSize(), 2);

  auto const_expr = reinterpret_cast<ConstantValueExpression *>(fun_expr->GetChild(0).get());
  EXPECT_NE(const_expr, nullptr);
  EXPECT_EQ(const_expr->GetValue().GetType(), type::TypeId::INTEGER);
  EXPECT_EQ(const_expr->GetValue().GetIntValue(), 1);

  auto tv_expr = reinterpret_cast<TupleValueExpression *>(fun_expr->GetChild(1).get());
  EXPECT_NE(tv_expr, nullptr);
  EXPECT_EQ(tv_expr->GetColumnName(), "b");
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, OldCaseTest) {
  std::string query = "SELECT id, case when id=100 then 1 else 0 end from tbl;";
  auto stmt_list = pgparser.BuildParseTree(query);
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldDateTypeTest) {
  // TODO(WAN): need typecast
  std::vector<std::string> valid_queries;
  valid_queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017-01-01'::DATE);");
  valid_queries.emplace_back("CREATE TABLE students (name TEXT, graduation DATE)");

  for (const auto &query : valid_queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }

  // Check invalid input handling
  std::vector<std::string> invalid_queries;
  invalid_queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017-00-01'::DATE);");
  invalid_queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017-01-011'::DATE);");
  invalid_queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017-00-'::DATE);");
  for (const auto &query : invalid_queries) {
    EXPECT_THROW(pgparser.BuildParseTree(query), ParserException);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldTypeCastTest) {
  // TODO(WAN): typecast support
  std::vector<std::string> queries;
  queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017'::INTEGER);");
  queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017'::FLOAT);");
  queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017'::DECIMAL);");
  queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017'::TEXT);");
  queries.emplace_back("INSERT INTO test_table VALUES (1, 2, '2017'::VARCHAR);");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

// NOLINTNEXTLINE
TEST_F(ParserTestBase, DISABLED_OldTypeCastInExpressionTest) {
  // TODO(WAN): typecast support
  std::vector<std::string> queries;
  queries.emplace_back("SELECT * FROM a WHERE d <= date '2018-04-04';");
  queries.emplace_back("SELECT '12345'::INTEGER - 12");

  for (const auto &query : queries) {
    auto stmt_list = pgparser.BuildParseTree(query);
  }
}

}  // namespace terrier::parser
