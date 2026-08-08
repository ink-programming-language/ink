// Translated from solution.cpp.

var k: dynamic;

var i: dynamic;

var n: dynamic;

var j: dynamic;

var ii: dynamic;

var jj: dynamic;

var f = cpp_array(51, 51);

var d = cpp_array(51, 51, 51, 51);

var val: dynamic;

var c = cpp_array(51, 51);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          read(c[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          f[i][j] = (((f[(i - 1)][j] + f[i][(j - 1)]) - f[(i - 1)][(j - 1)]) + ((c[i][j] == cpp_char("#"))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var check = __cpp_lambda_1;
  {
    i = n;
    while ((i >= 1))
    {
      {
        j = n;
        while ((j >= 1))
        {
          {
            ii = i;
            while ((ii <= n))
            {
              {
                jj = j;
                while ((jj <= n))
                {
                  if ((check(i, j, ii, jj) == 0))
                  {
                    d[i][j][ii][jj] = 0;
                    jj += 1;
                    continue;
                  }
                  val = max(((jj - j) + 1), ((ii - i) + 1));
                  {
                    k = j;
                    while ((k < jj))
                    {
                      val = min(val, (d[i][j][ii][k] + d[i][(k + 1)][ii][jj]));
                      k += 1;
                    }
                  }
                  {
                    k = i;
                    while ((k < ii))
                    {
                      val = min(val, (d[i][j][k][jj] + d[(k + 1)][j][ii][jj]));
                      k += 1;
                    }
                  }
                  d[i][j][ii][jj] = val;
                  jj += 1;
                }
              }
              ii += 1;
            }
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  write(d[1][1][n][n]);
}

func __cpp_lambda_1(x: dynamic, y: dynamic, xx: dynamic, yy: dynamic)
{
  return (((f[xx][yy] - f[(x - 1)][yy]) - f[xx][(y - 1)]) + f[(x - 1)][(y - 1)]);
}
