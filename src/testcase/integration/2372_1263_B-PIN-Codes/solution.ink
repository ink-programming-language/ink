// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var v: dynamic = cpp_uninitialized();
    var ms: dynamic = cpp_uninitialized();
    var st: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var s: dynamic = cpp_uninitialized();
        read(s);
        v.push_back(s);
        ms.insert(s);
        st.insert(s);
        i += 1;
      }
    }
    var ans: dynamic = 0;
    for (var u: dynamic in st)
    {
      ans += (ms.count(u) - 1);
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((ms.count(v[i]) == 1))
        {
          i += 1;
          continue;
        }
        var it: dynamic = ms.find(v[i]);
        ms.erase(it);
        {
          var j: dynamic = cpp_char("0");
          while ((j <= cpp_char("9")))
          {
            v[i][0] = char(j);
            var t: dynamic = true;
            {
              var z: dynamic = 0;
              while ((z < n))
              {
                if ((z == i))
                {
                  z += 1;
                  continue;
                }
                if ((v[z] == v[i]))
                {
                  t = false;
                  break;
                }
                z += 1;
              }
            }
            if (t)
            {
              break;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(ans, "\n");
    for (var u: dynamic in v)
    {
      write(u, "\n");
    }
  }
  return 0;
}
