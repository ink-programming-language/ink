// Translated from solution.cpp.

var maxn = 105;

func gi()
{
  var c = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    c = getchar();
  }
  var sum = 0;
  while (((cpp_char("0") <= c) && (c <= cpp_char("9"))))
  {
    sum = (((sum * 10) + c) - 48);
    c = getchar();
  }
  return sum;
}

var n: dynamic;

var A = cpp_array(maxn, maxn, maxn);

func main()
{
  n = gi();
  if ((n == 1))
  {
    return cpp_comma(puts("1"), 0);
  }
  A[1][1][1] = 4;
  A[1][1][2] = 1;
  A[1][2][1] = 3;
  A[1][2][2] = 2;
  A[2][1][1] = 5;
  A[2][1][2] = 8;
  A[2][2][1] = 6;
  A[2][2][2] = 7;
  A[3][1][1] = 10;
  A[3][1][2] = 9;
  A[3][2][1] = 11;
  A[3][2][2] = 12;
  {
    var i = 3;
    while ((i <= n))
    {
      if ((i & 1))
      {
        var p1 = 1;
        {
          var j = 1;
          while ((j <= i))
          {
            A[j][1][i] = cpp_update(p1, "--");
            j += 1;
          }
        }
        {
          var j = i;
          while (j)
          {
            if ((((j - i)) & 1))
            {
              {
                var k = 1;
                while ((k <= i))
                {
                  A[j][0][k] = cpp_update(p1, "--");
                  k += 1;
                }
              }
            } else
            {
              {
                var k = i;
                while (k)
                {
                  A[j][0][k] = cpp_update(p1, "--");
                  k -= 1;
                }
              }
            }
            j -= 1;
          }
        }
        var p2 = A[i][(i - 1)][(i - 1)];
        {
          var j = (i - 1);
          while ((j >= 2))
          {
            A[i][j][i] = cpp_update(p2, "++");
            j -= 1;
          }
        }
        {
          var j = 2;
          while ((j < i))
          {
            if ((j & 1))
            {
              {
                var k = 1;
                while ((k < i))
                {
                  A[k][j][i] = cpp_update(p2, "++");
                  k += 1;
                }
              }
            } else
            {
              {
                var k = (i - 1);
                while (k)
                {
                  A[k][j][i] = cpp_update(p2, "++");
                  k -= 1;
                }
              }
            }
            j += 1;
          }
        }
        {
          var j = 1;
          while ((j <= i))
          {
            if ((j & 1))
            {
              {
                var k = i;
                while (k)
                {
                  A[j][i][k] = cpp_update(p2, "++");
                  k -= 1;
                }
              }
            } else
            {
              {
                var k = 1;
                while ((k <= i))
                {
                  A[j][i][k] = cpp_update(p2, "++");
                  k += 1;
                }
              }
            }
            j += 1;
          }
        }
        {
          var p = (i + 1);
          while (p)
          {
            {
              var x = 1;
              while ((x <= i))
              {
                {
                  var y = 1;
                  while ((y <= i))
                  {
                    A[x][p][y] = ((A[x][(p - 1)][y] - p1) + 1);
                    y += 1;
                  }
                }
                x += 1;
              }
            }
            p -= 1;
          }
        }
        {
          var x = 1;
          while ((x <= i))
          {
            {
              var y = 1;
              while ((y <= (i + 1)))
              {
                {
                  var z = 1;
                  while (((z * 2) <= i))
                  {
                    swap(A[x][y][z], A[x][y][((i + 1) - z)]);
                    z += 1;
                  }
                }
                y += 1;
              }
            }
            x += 1;
          }
        }
      } else
      {
        var p1 = 1;
        {
          var j = 1;
          while ((j <= i))
          {
            A[1][j][i] = cpp_update(p1, "--");
            j += 1;
          }
        }
        {
          var j = i;
          while (j)
          {
            if ((((i - j)) & 1))
            {
              {
                var k = 1;
                while ((k <= i))
                {
                  A[0][j][k] = cpp_update(p1, "--");
                  k += 1;
                }
              }
            } else
            {
              {
                var k = i;
                while (k)
                {
                  A[0][j][k] = cpp_update(p1, "--");
                  k -= 1;
                }
              }
            }
            j -= 1;
          }
        }
        var p2 = A[(i - 1)][i][(i - 1)];
        {
          var j = i;
          while (j)
          {
            if ((((i - j)) & 1))
            {
              {
                var k = 2;
                while ((k < i))
                {
                  A[k][j][i] = cpp_update(p2, "++");
                  k += 1;
                }
              }
            } else
            {
              {
                var k = (i - 1);
                while ((k >= 2))
                {
                  A[k][j][i] = cpp_update(p2, "++");
                  k -= 1;
                }
              }
            }
            j -= 1;
          }
        }
        {
          var j = 1;
          while ((j <= i))
          {
            if ((j & 1))
            {
              {
                var k = i;
                while (k)
                {
                  A[i][j][k] = cpp_update(p2, "++");
                  k -= 1;
                }
              }
            } else
            {
              {
                var k = 1;
                while ((k <= i))
                {
                  A[i][j][k] = cpp_update(p2, "++");
                  k += 1;
                }
              }
            }
            j += 1;
          }
        }
        {
          var p = (i + 1);
          while (p)
          {
            {
              var x = 1;
              while ((x <= i))
              {
                {
                  var y = 1;
                  while ((y <= i))
                  {
                    A[p][x][y] = ((A[(p - 1)][x][y] - p1) + 1);
                    y += 1;
                  }
                }
                x += 1;
              }
            }
            p -= 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var y = 1;
    while ((y <= n))
    {
      {
        var z = n;
        while (z)
        {
          {
            var x = 1;
            while ((x <= n))
            {
              printf("%d ", A[x][y][z]);
              x += 1;
            }
          }
          puts("");
          z -= 1;
        }
      }
      puts("");
      y += 1;
    }
  }
  return 0;
}
