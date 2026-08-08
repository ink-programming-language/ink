// Translated from solution.cpp.

func solver(x: dynamic, v: dynamic, n: dynamic)
{
  var idx = -1;
  var i: dynamic;
  {
    i = 0;
    while ((i < (n - 1)))
    {
      if ((v[i].find(x) != v[i].end()))
      {
        v[i].erase(x);
      }
      if ((cpp_cast(v[i].size()) == 1))
      {
        if ((idx == -1))
        {
          idx = i;
        } else
        {
          return -1;
        }
      }
      i += 1;
    }
  }
  return idx;
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var i: dynamic;
    var j: dynamic;
    var k: dynamic;
    var l: dynamic;
    read(n);
    var v = cpp_array((n - 1));
    {
      i = 0;
      while ((i < (n - 1)))
      {
        var k: dynamic;
        read(k);
        {
          j = 0;
          while ((j < k))
          {
            var y: dynamic;
            read(y);
            v[i].push_back(y);
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
        var ans: dynamic;
        var s = cpp_construct((n - 1));
        {
          j = 0;
          while ((j < (n - 1)))
          {
            var si = v[j].size();
            {
              k = 0;
              while ((k < si))
              {
                s[j].insert(v[j][k]);
                k += 1;
              }
            }
            j += 1;
          }
        }
        var crr = i;
        {
          j = 0;
          while ((j < n))
          {
            ans.push_back(crr);
            if ((j < (n - 1)))
            {
              var tt = solver(crr, s, n);
              if ((tt == -1))
              {
                break;
              } else
              {
                crr = (*s[tt].begin());
              }
            }
            j += 1;
          }
        }
        if ((j != n))
        {
          i += 1;
          continue;
        }
        var used = cpp_construct((n - 1), 0);
        {
          j = 1;
          while ((j < n))
          {
            var temp: dynamic;
            temp.insert(ans[j]);
            {
              k = (j - 1);
              while ((k >= 0))
              {
                temp.insert(ans[k]);
                {
                  l = 0;
                  while ((l < (n - 1)))
                  {
                    if ((used[l] == 0))
                    {
                      var si_v = v[l].size();
                      if ((si_v == (((j - k) + 1))))
                      {
                        var it: dynamic;
                        var xx = 0;
                        {
                          it = temp.begin();
                          while ((it != temp.end()))
                          {
                            if ((v[l][xx] != (*it)))
                            {
                              break;
                            }
                            it += 1;
                            xx += 1;
                          }
                        }
                        if ((it == temp.end()))
                        {
                          used[l] = 1;
                          break;
                        }
                      }
                    }
                    l += 1;
                  }
                }
                if ((l != (n - 1)))
                {
                  break;
                }
                k -= 1;
              }
            }
            if ((k != -1))
            {
              j += 1;
              continue;
            } else
            {
              break;
            }
            j += 1;
          }
        }
        if ((j != n))
        {
          i += 1;
          continue;
        } else
        {
          {
            j = 0;
            while ((j < n))
            {
              write(ans[j], " ");
              j += 1;
            }
          }
          write("\n");
          break;
        }
        i += 1;
      }
    }
  }
}
