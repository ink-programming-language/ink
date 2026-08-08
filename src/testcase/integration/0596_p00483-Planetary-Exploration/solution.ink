// Translated from solution.cpp.

var exist = [];

var field = cpp_array(1001, 1001);

var m: dynamic;

var n: dynamic;

var Q: dynamic;

func main()
{
  scanf("%d %d %d", (&m), (&n), (&Q));
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%s", field[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = (i + 1);
        while ((j <= m))
        {
          if ((field[i][0] == cpp_char("J")))
          {
            exist[j][1][0] += 1;
          } else if ((field[i][0] == cpp_char("O")))
          {
            exist[j][1][1] += 1;
          } else
          {
            exist[j][1][2] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j <= n))
        {
          if ((field[0][i] == cpp_char("J")))
          {
            exist[1][j][0] += 1;
          } else if ((field[0][i] == cpp_char("O")))
          {
            exist[1][j][1] += 1;
          } else
          {
            exist[1][j][2] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((field[0][0] == cpp_char("J")))
  {
    exist[1][1][0] = 1;
  } else if ((field[0][0] == cpp_char("O")))
  {
    exist[1][1][1] = 1;
  } else
  {
    exist[1][1][2] = 1;
  }
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              exist[i][j][k] = exist[i][(j - 1)][k];
              k += 1;
            }
          }
          {
            var k = 0;
            while ((k < i))
            {
              if ((field[k][(j - 1)] == cpp_char("J")))
              {
                exist[i][j][0] += 1;
              } else if ((field[k][(j - 1)] == cpp_char("O")))
              {
                exist[i][j][1] += 1;
              } else
              {
                exist[i][j][2] += 1;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Q))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      var d: dynamic;
      scanf("%d %d %d %d", (&a), (&b), (&c), (&d));
      var J = (((exist[c][d][0] - exist[c][(b - 1)][0]) - exist[(a - 1)][d][0]) + exist[(a - 1)][(b - 1)][0]);
      var O = (((exist[c][d][1] - exist[c][(b - 1)][1]) - exist[(a - 1)][d][1]) + exist[(a - 1)][(b - 1)][1]);
      printf("%d %d %d\n", J, O, ((((((c - a) + 1)) * (((1 + d) - b))) - J) - O));
      i += 1;
    }
  }
  return 0;
}
