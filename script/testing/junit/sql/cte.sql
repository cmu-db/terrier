-- Input queries for vanilla (simple) CTE integration tests.

-----------------------------------------------------------
-- Hand-Rolled Test Cases

WITH cte(x) AS (SELECT 1) SELECT x FROM cte;
WITH cte AS (SELECT 1) SELECT * FROM cte;
WITH cte AS (SELECT 1 AS "x") SELECT * FROM cte;
WITH cte(y) AS (SELECT 1 AS "x") SELECT y FROM cte;
WITH cte(x) AS (WITH cte2(y) AS (SELECT 1) SELECT y FROM cte2) SELECT x FROM cte;

-- TODO: We currently fail on this query because of the arbitrary SELECT * ordering
-- WITH cte(x) AS (SELECT 1), cte2(y) AS (SELECT 2) SELECT * FROM cte INNER JOIN cte2 ON cte.x+1 = cte2.y;

WITH cte(x) AS (SELECT 1), cte2(y) AS (SELECT 2) SELECT cte.x, cte2.y FROM cte INNER JOIN cte2 ON cte.x+1 = cte2.y;

WITH cte(x,x) AS (SELECT 1, 2) SELECT * FROM cte;
WITH cte AS (SELECT 4, 3) SELECT * FROM cte;
WITH cte(y,y,x) AS (SELECT 5,4,3) SELECT x FROM cte;

-- Port of some JUnit test cases
CREATE TABLE company (id INT PRIMARY KEY NOT NULL, name TEXT NOT NULL, age INT NOT NULL, address CHAR(50), salary REAL);
INSERT INTO company (id,name,age,address,salary) VALUES (1, 'Paul', 32, 'California', 20000.00);
INSERT INTO company (id,name,age,address,salary) VALUES (2, 'George', 21, 'NY', 10000.00);

WITH employee AS (SELECT id, name, age FROM company) SELECT name FROM employee;
WITH employee AS (SELECT age+age AS sumage, name FROM company) SELECT E2.name, E1.sumage FROM employee AS E1, employee AS E2 WHERE E1.name = E2.name;

-- Aggregate inside CTE query
WITH employee AS (SELECT SUM(age) AS sumage FROM company) SELECT * FROM employee;

-- Join inside CTE query
WITH employee AS (SELECT C1.name AS name, C2.age AS age FROM company AS C1, company AS C2) SELECT * FROM employee;

-- Aggregate with alias inside CTE query
WITH employee AS (SELECT MAX(age) AS mxage FROM company) SELECT E2.name, E2.age FROM employee AS E1, company AS E2 WHERE E1.mxage = E2.age;

CREATE TABLE tmp(x INT);
INSERT INTO tmp VALUES (1), (2);

-- CTE with self-join
WITH cte AS (SELECT * FROM tmp) SELECT A.x, B.x, C.x FROM cte A, cte B, tmp C ORDER BY A.x, B.x, C.x;
WITH cte AS (SELECT * FROM tmp) SELECT A.x, B.x, C.x FROM cte A, cte B, cte C ORDER BY A.x, B.x, C.x;

-- Infinite loop, but outer query limits the number of rows
-- TODO: We loop on this
-- WITH RECURSIVE i(x) AS (SELECT 1 UNION SELECT (x+1)%10 FROM i) SELECT x FROM i LIMIT 20;

-- Infinite loop, but outer query limits the number of rows
-- TODO: We loop on this
-- WITH RECURSIVE i(x) AS (SELECT 1 UNION ALL SELECT (x+1)%10 FROM i) SELECT x FROM i LIMIT 20;

-----------------------------------------------------------
-- Adapted from Postgres Regression Test (`with.sql`)
-- https://github.com/postgres/postgres/blob/master/src/test/regress/sql/with.sql

WITH q1(x,y) AS (SELECT 1,2) SELECT * FROM q1, q1 AS q2;
