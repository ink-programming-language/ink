// Translated from solution.cpp.

var board: dynamic = cpp_array(4, 4);

func Valid(x: dynamic, y: dynamic) -> dynamic
{
  return (cpp_assign(((x >= 0) && x), "=", (0 && (y < 4))));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      {
        var j: dynamic = 0;
        while ((j < 4))
        {
          read(board[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var valid: dynamic = false;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      {
        var j: dynamic = 0;
        while ((j < 4))
        {
          if ((board[i][j] == cpp_char("x")))
          {
            if (((((Valid((i - 1), j) && (board[(i - 1)][j] == cpp_char("x"))) && Valid((i - 2), j)) && (board[(i - 2)][j] == cpp_char("."))) || (((Valid((i - 1), j) && (board[(i - 1)][j] == cpp_char("."))) && Valid((i - 2), j)) && (board[(i - 2)][j] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid((i + 1), j) && (board[(i + 1)][j] == cpp_char("x"))) && Valid((i + 2), j)) && (board[(i + 2)][j] == cpp_char("."))) || (((Valid((i + 1), j) && (board[(i + 1)][j] == cpp_char("."))) && Valid((i + 2), j)) && (board[(i + 2)][j] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid(i, (j - 1)) && (board[i][(j - 1)] == cpp_char("x"))) && Valid(i, (j - 2))) && (board[i][(j - 2)] == cpp_char("."))) || (((Valid(i, (j - 1)) && (board[i][(j - 1)] == cpp_char("."))) && Valid(i, (j - 2))) && (board[i][(j - 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid(i, (j + 1)) && (board[i][(j + 1)] == cpp_char("x"))) && Valid(i, (j + 2))) && (board[i][(j + 2)] == cpp_char("."))) || (((Valid(i, (j + 1)) && (board[i][(j + 1)] == cpp_char("."))) && Valid(i, (j + 2))) && (board[i][(j + 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid((i + 1), (j + 1)) && (board[(i + 1)][(j + 1)] == cpp_char("x"))) && Valid((i + 2), (j + 2))) && (board[(i + 2)][(j + 2)] == cpp_char("."))) || (((Valid((i + 1), (j + 1)) && (board[(i + 1)][(j + 1)] == cpp_char("."))) && Valid((i + 2), (j + 2))) && (board[(i + 2)][(j + 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid((i - 1), (j - 1)) && (board[(i - 1)][(j - 1)] == cpp_char("x"))) && Valid((i - 2), (j - 2))) && (board[(i - 2)][(j - 2)] == cpp_char("."))) || (((Valid((i - 1), (j - 1)) && (board[(i - 1)][(j - 1)] == cpp_char("."))) && Valid((i - 2), (j - 2))) && (board[(i - 2)][(j - 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid((i - 1), (j + 1)) && (board[(i - 1)][(j + 1)] == cpp_char("x"))) && Valid((i - 2), (j + 2))) && (board[(i - 2)][(j + 2)] == cpp_char("."))) || (((Valid((i - 1), (j + 1)) && (board[(i - 1)][(j + 1)] == cpp_char("."))) && Valid((i - 2), (j + 2))) && (board[(i - 2)][(j + 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            } else if (((((Valid((i + 1), (j - 1)) && (board[(i + 1)][(j - 1)] == cpp_char("x"))) && Valid((i + 2), (j - 2))) && (board[(i + 2)][(j - 2)] == cpp_char("."))) || (((Valid((i + 1), (j - 1)) && (board[(i + 1)][(j - 1)] == cpp_char("."))) && Valid((i + 2), (j - 2))) && (board[(i + 2)][(j - 2)] == cpp_char("x")))))
            {
              valid = true;
              break;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
   ((valid)) ? (cout << "YES\n") : (cout << "NO\n");
  return 0;
}
