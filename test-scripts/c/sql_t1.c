// libssl-dev libmariadbclient-dev
// gcc sql_t1.c -c -I/usr/include/mysql/ -L/lib/x86_64-linux-gnu/ -lmariadbclient -lz -lcrypt -lnsl -lm -lpthread -lssl -lcrypto -ldl `mysql_config --cflags --libs`

#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
//#include <pthread.h>

#include "mariadb_interaction.h"
#include "sql_t1.h"

/**
Executes SQL query 'sql' using 'conn' connection and print results
*/
int execute_select_query_and_check(MYSQL *conn, char *sql, unsigned long long int rows)
{
     MYSQL_RES *res;
     MYSQL_ROW row;
     unsigned long long int i;
     unsigned long long int num_fields;
     unsigned long long int int_res;
     unsigned long long int row_i=0;
     int test_result = 0;

     printf("Trying SELECT, num_of_rows=%llu\n", rows);
     if(mysql_query(conn, sql) != 0)
         printf("Error: can't execute SQL-query: %s\n", mysql_error(conn));

     res = mysql_store_result(conn);
     if(res == NULL) printf("Error: can't get the result description\n");
    printf("rows=%llu\n", mysql_num_rows(res));
    if (mysql_num_rows(res) != rows) {printf("SELECT returned %llu rows insted of %llu!\n", mysql_num_rows(res), rows); test_result=1;  printf("sql was %s\n", sql);}
    num_fields = mysql_num_fields(res);
    if (num_fields != 2) { printf("SELECT returned %llu fileds insted of 2!\n", num_fields); test_result=1; }
    if(mysql_num_rows(res) > 0)
    {
      while((row = mysql_fetch_row(res)) != NULL) { 
        for (i = 0; i < num_fields; i++) {
//          printf("%s\t", row[i]);
	  sscanf(row[i], "%llu", &int_res); 
          if ((i == 0 ) && (int_res != row_i)) {printf("SELECT returned wrong result! %llu insted of expected %llu\n", int_res, row_i); test_result=1; printf("sql was %s\n", sql);}
        }
//        printf("\n"); 
        row_i++;
      }
    }

      mysql_free_result(res); 
      return(test_result);
}
int create_t1(MYSQL * conn)
{
  execute_query(conn, "DROP TABLE IF EXISTS t1;");  
  printf("Creating test table\n");
  execute_query(conn, "CREATE TABLE t1 (x1 int, fl int);"); 
}

int create_insert_string(char *sql, int N, int fl)
{
  char *ins1 = "INSERT INTO t1 (x1, fl) VALUES ";
  char *ins_val="%s (%d, %d)%s";
  int i;

  sprintf(&sql[0], "%s", ins1);
  for (i = 0; i < N-1; i++) {
    sprintf(&sql[0], ins_val, sql, i, fl, ",");
  }
  sprintf(&sql[0], ins_val, sql, N-1, fl, ";");
}

int insert_into_t1(MYSQL *conn, int N)
{
  char sql[N][1000000];
  int x=16;
  int i;
  char *ins1 = "INSERT INTO t1 (x1, fl) VALUES ";
  char *ins_val="%s (%d, 1)%s";

  printf("Generating long INSERTs\n");
  for (i=0; i<N; i++) {
    printf("sql %d, rows=%d\n", i, x);
    create_insert_string(sql[i], x, i);
    x = x*16;
    printf("INSERT: rwsplitter\n");
    printf("Trying INSERT, len=%lu\n", strlen(sql[i]));
    fflush(stdout);
    execute_query(conn,  sql[i]);
    fflush(stdout);
  }
}

int select_from_t1(MYSQL *conn, int N)
{
  int x=16;
  int result=0;
  int i;
  char sq[100];

  for (i=0; i<N; i++) {
    sprintf(&sq[0], "select * from t1 where fl=%d;", i);
    result += execute_select_query_and_check(conn, sq, x);
    x = x * 16;
  }
  return(result);
}

int check_if_t1_exists(MYSQL *conn)
{
   MYSQL_RES *res;
   MYSQL_ROW row;
   unsigned long long int num_fields;

   int t1 = 0;
   if (mysql_query(conn, "show tables;") != 0) {
         printf("Error: can't execute SQL-query: %s\n", mysql_error(conn));
         t1 = 0;
    } else {
        res = mysql_store_result(conn);
         if (res == NULL) printf("Error: can't get the result description\n");

//        printf("number of tables=%llu\n", mysql_num_rows(res));
        num_fields = mysql_num_fields(res);


        if(mysql_num_rows(res) > 0)
        {
          while((row = mysql_fetch_row(res)) != NULL) { 
            if ( (row[0] != NULL ) && (strcmp(row[0], "t1") ==0 ) ) {
 		t1 = 1;
            }
          }
        }
   }
  return(t1);
}
