// Translated from solution.cpp.

func solver(x: dynamic, v: dynamic, n: dynamic) -> dynamic
{
  var idx: dynamic = -1;
  var i: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    read(n);
    var v: dynamic = cpp_array((n - 1));
    {
      i = 0;
      while ((i < (n - 1)))
      {
        var k: dynamic = cpp_uninitialized();
        read(k);
        {
          j = 0;
          while ((j < k))
          {
            var y: dynamic = cpp_uninitialized();
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
        var ans: dynamic = cpp_uninitialized();
        var s: dynamic = cpp_construct((n - 1));
        {
          j = 0;
          while ((j < (n - 1)))
          {
            var si: dynamic = v[j].size();
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
        var crr: dynamic = i;
        {
          j = 0;
          while ((j < n))
          {
            ans.push_back(crr);
            if ((j < (n - 1)))
            {
              var tt: dynamic = solver(crr, s, n);
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
        var used: dynamic = cpp_construct((n - 1), 0);
        {
          j = 1;
          while ((j < n))
          {
            var temp: dynamic = cpp_uninitialized();
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
                      var si_v: dynamic = v[l].size();
                      if ((si_v == (((j - k) + 1))))
                      {
                        var it: dynamic = cpp_uninitialized();
                        var xx: dynamic = 0;
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
