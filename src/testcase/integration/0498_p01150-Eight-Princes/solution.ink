// Translated from solution.cpp.

var sum: dynamic = 0;

func solve(used: dynamic, nums: dynamic) -> dynamic
{
  if ((used.count() == 13))
  {
    return [-1];
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < 13))
      {
        if ((!used[i]))
        {
          if (((sum % ((i + 1))) == 0))
          {
            sum += (((i + 1)) * nums[i]);
            used[i] = true;
            if ((!v.empty()))
            {
              if ((v == [-1]))
              {
                v.clear();
              }
              {
                var j: dynamic = 0;
                while ((j < nums[i]))
                {
                  v.push_back(i);
                  j += 1;
                }
              }
              return v;
            }
            used[i] = false;
            sum -= (((i + 1)) * nums[i]);
          }
        }
        i += 1;
      }
    }
    return vector();
  }
}

func main() -> dynamic
{
  while (true)
  {
    sum = 0;
    var N: dynamic = cpp_uninitialized();
    read(N);
    if ((!N))
    {
      break;
    }
    if ((N % 2))
    {
      var dp: dynamic = cpp_array(9, 2, 2, 100);
      {
        var i: dynamic = 0;
        while ((i < 100))
        {
          {
            var j: dynamic = 0;
            while ((j < 2))
            {
              {
                var k: dynamic = 0;
                while ((k < 2))
                {
                  {
                    var n: dynamic = 0;
                    while ((n < 9))
                    {
                      dp[i][j][k][n] = 0;
                      n += 1;
                    }
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
        var luse: dynamic = 0;
        while ((luse < 2))
        {
          dp[0][luse][luse][luse] = 1;
          luse += 1;
        }
      }
      {
        var h: dynamic = 0;
        while ((h < (N - 1)))
        {
          {
            var fst_luse: dynamic = 0;
            while ((fst_luse < 2))
            {
              {
                var pre_luse: dynamic = 0;
                while ((pre_luse < 2))
                {
                  {
                    var cnt: dynamic = 0;
                    while ((cnt <= 8))
                    {
                      var num: dynamic = dp[h][fst_luse][pre_luse][cnt];
                      {
                        var next_luse: dynamic = 0;
                        while ((next_luse < 2))
                        {
                          var next_cnt: dynamic = (cnt + next_luse);
                          if ((next_cnt > 8))
                          {
                            next_luse += 1;
                            continue;
                          }
                          var next_h: dynamic = (h + 1);
                          if ((pre_luse && next_luse))
                          {
                            next_luse += 1;
                            continue;
                          }
                          if ((next_h == (N - 1)))
                          {
                            if ((fst_luse && next_luse))
                            {
                              next_luse += 1;
                              continue;
                            }
                          }
                          dp[next_h][fst_luse][next_luse][next_cnt] += num;
                          next_luse += 1;
                        }
                      }
                      cnt += 1;
                    }
                  }
                  pre_luse += 1;
                }
              }
              fst_luse += 1;
            }
          }
          h += 1;
        }
      }
      var ans: dynamic = 0;
      {
        var h: dynamic = (N - 1);
        {
          var fst_luse: dynamic = 0;
          while ((fst_luse < 2))
          {
            {
              var pre_luse: dynamic = 0;
              while ((pre_luse < 2))
              {
                {
                  var cnt: dynamic = 8;
                  var num: dynamic = dp[h][fst_luse][pre_luse][cnt];
                  ans += num;
                }
                pre_luse += 1;
              }
            }
            fst_luse += 1;
          }
        }
      }
      ans *= 40320;
      write(ans, "\n");
    } else
    {
      var dp: dynamic = cpp_array(9, 2, 2, 2, 2, 100);
      {
        var i: dynamic = 0;
        while ((i < 100))
        {
          {
            var j: dynamic = 0;
            while ((j < 2))
            {
              {
                var k: dynamic = 0;
                while ((k < 2))
                {
                  {
                    var l: dynamic = 0;
                    while ((l < 2))
                    {
                      {
                        var m: dynamic = 0;
                        while ((m < 2))
                        {
                          {
                            var n: dynamic = 0;
                            while ((n < 9))
                            {
                              dp[i][j][k][l][m][n] = 0;
                              n += 1;
                            }
                          }
                          m += 1;
                        }
                      }
                      l += 1;
                    }
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
        var luse: dynamic = 0;
        while ((luse < 2))
        {
          {
            var ruse: dynamic = 0;
            while ((ruse < 2))
            {
              if ((luse && ruse))
              {
                ruse += 1;
                continue;
              }
              dp[0][luse][ruse][luse][ruse][(luse + ruse)] = 1;
              ruse += 1;
            }
          }
          luse += 1;
        }
      }
      {
        var h: dynamic = 0;
        while ((h < ((N / 2) - 1)))
        {
          {
            var fst_luse: dynamic = 0;
            while ((fst_luse < 2))
            {
              {
                var fst_ruse: dynamic = 0;
                while ((fst_ruse < 2))
                {
                  {
                    var pre_luse: dynamic = 0;
                    while ((pre_luse < 2))
                    {
                      {
                        var pre_ruse: dynamic = 0;
                        while ((pre_ruse < 2))
                        {
                          {
                            var cnt: dynamic = 0;
                            while ((cnt <= 8))
                            {
                              var num: dynamic = dp[h][fst_luse][fst_ruse][pre_luse][pre_ruse][cnt];
                              {
                                var next_luse: dynamic = 0;
                                while ((next_luse < 2))
                                {
                                  {
                                    var next_ruse: dynamic = 0;
                                    while ((next_ruse < 2))
                                    {
                                      var next_cnt: dynamic = ((cnt + next_luse) + next_ruse);
                                      if ((next_cnt > 8))
                                      {
                                        next_ruse += 1;
                                        continue;
                                      }
                                      var next_h: dynamic = (h + 1);
                                      if ((pre_luse && next_ruse))
                                      {
                                        next_ruse += 1;
                                        continue;
                                      }
                                      if ((pre_ruse && next_luse))
                                      {
                                        next_ruse += 1;
                                        continue;
                                      }
                                      if ((next_luse && next_ruse))
                                      {
                                        next_ruse += 1;
                                        continue;
                                      }
                                      if ((next_h == ((N / 2) - 1)))
                                      {
                                        if ((fst_luse && next_luse))
                                        {
                                          next_ruse += 1;
                                          continue;
                                        }
                                        if ((fst_ruse && next_ruse))
                                        {
                                          next_ruse += 1;
                                          continue;
                                        }
                                      }
                                      dp[next_h][fst_luse][fst_ruse][next_luse][next_ruse][next_cnt] += num;
                                      next_ruse += 1;
                                    }
                                  }
                                  next_luse += 1;
                                }
                              }
                              cnt += 1;
                            }
                          }
                          pre_ruse += 1;
                        }
                      }
                      pre_luse += 1;
                    }
                  }
                  fst_ruse += 1;
                }
              }
              fst_luse += 1;
            }
          }
          h += 1;
        }
      }
      var ans: dynamic = 0;
      {
        var h: dynamic = ((N / 2) - 1);
        {
          var fst_luse: dynamic = 0;
          while ((fst_luse < 2))
          {
            {
              var fst_ruse: dynamic = 0;
              while ((fst_ruse < 2))
              {
                {
                  var pre_luse: dynamic = 0;
                  while ((pre_luse < 2))
                  {
                    {
                      var pre_ruse: dynamic = 0;
                      while ((pre_ruse < 2))
                      {
                        {
                          var cnt: dynamic = 8;
                          var num: dynamic = dp[h][fst_luse][fst_ruse][pre_luse][pre_ruse][cnt];
                          ans += num;
                        }
                        pre_ruse += 1;
                      }
                    }
                    pre_luse += 1;
                  }
                }
                fst_ruse += 1;
              }
            }
            fst_luse += 1;
          }
        }
      }
      ans *= 40320;
      write(ans, "\n");
    }
  }
  return 0;
}
