// Translated from solution.cpp.

var M = 1000000;

var n: dynamic;

var sch = cpp_array(20, 400);

var dp = cpp_array(M, 2);

var dy = [0, -1, 0, 1, 0];

var dx = [0, 0, 1, 0, -1];

var table = cpp_array(5, 5);

var y: dynamic;

var x: dynamic;

func main()
{
  while (cpp_comma((cin >> n), n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < 16))
          {
            read(sch[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < M))
      {
        dp[0][i] = false;
        i += 1;
      }
    }
    dp[0][4] = ((((!sch[0][5]) && (!sch[0][6])) && (!sch[0][9])) && (!sch[0][10]));
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        var now = (i & 1);
        var nxt = (1 - now);
        {
          var j = 0;
          while ((j < M))
          {
            dp[nxt][j] = false;
            j += 1;
          }
        }
        {
          var j = 0;
          while ((j < M))
          {
            if (dp[now][j])
            {
              {
                var ii = 0;
                while ((ii < 4))
                {
                  {
                    var jj = 0;
                    while ((jj < 4))
                    {
                      table[ii][jj] = (if (((i < 5))) true else false);
                      jj += 1;
                    }
                  }
                  ii += 1;
                }
              }
              var tmp = j;
              {
                var z = 0;
                while ((z < 6))
                {
                  var pos = (tmp % 10);
                  y = (pos / 3);
                  x = (pos % 3);
                  {
                    var ii = 0;
                    while ((ii <= 1))
                    {
                      {
                        var jj = 0;
                        while ((jj <= 1))
                        {
                          table[(y + ii)][(x + jj)] = true;
                          jj += 1;
                        }
                      }
                      ii += 1;
                    }
                  }
                  tmp /= 10;
                  z += 1;
                }
              }
              tmp = (j % 10);
              y = (tmp / 3);
              x = (tmp % 3);
              {
                var ii = 1;
                while ((ii <= 2))
                {
                  {
                    var jj = 0;
                    while ((jj <= 4))
                    {
                      var f = true;
                      var ny = (y + (ii * dy[jj]));
                      var nx = (x + (ii * dx[jj]));
                      if (((((ny < 0) || (nx < 0)) || (ny >= 3)) || (nx >= 3)))
                      {
                        jj += 1;
                        continue;
                      }
                      var piyo = cpp_array(5, 5);
                      {
                        var iii = 0;
                        while ((iii < 4))
                        {
                          {
                            var jjj = 0;
                            while ((jjj < 4))
                            {
                              piyo[iii][jjj] = table[iii][jjj];
                              jjj += 1;
                            }
                          }
                          iii += 1;
                        }
                      }
                      {
                        var iii = 0;
                        while ((iii <= 1))
                        {
                          {
                            var jjj = 0;
                            while ((jjj <= 1))
                            {
                              if (sch[(i + 1)][(((((ny + iii)) * 4) + nx) + jjj)])
                              {
                                f = false;
                                cpp_goto("goto END;");
                              }
                              piyo[(ny + iii)][(nx + jjj)] = true;
                              jjj += 1;
                            }
                          }
                          iii += 1;
                        }
                      }
                      {
                        var iii = 0;
                        while ((iii < 4))
                        {
                          {
                            var jjj = 0;
                            while ((jjj < 4))
                            {
                              if ((!piyo[iii][jjj]))
                              {
                                f = false;
                                cpp_goto("goto END;");
                              }
                              jjj += 1;
                            }
                          }
                          iii += 1;
                        }
                      }
                      dp[nxt][((((j * 10) + (((ny * 3) + nx)))) % M)] |= f;
                      jj += 1;
                    }
                  }
                  ii += 1;
                }
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ans = false;
    {
      var i = 0;
      while ((i < M))
      {
        ans |= dp[(((n - 1)) & 1)][i];
        i += 1;
      }
    }
    if (ans)
    {
      write("1\n");
    } else
    {
      write("0\n");
    }
  }
}
