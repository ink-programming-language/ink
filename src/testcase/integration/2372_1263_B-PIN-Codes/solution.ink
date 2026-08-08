// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var v: dynamic;
    var ms: dynamic;
    var st: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        var s: dynamic;
        read(s);
        v.push_back(s);
        ms.insert(s);
        st.insert(s);
        i += 1;
      }
    }
    var ans = 0;
    for (var u in st)
    {
      ans += (ms.count(u) - 1);
    }
    {
      var i = 0;
      while ((i < n))
      {
        if ((ms.count(v[i]) == 1))
        {
          i += 1;
          continue;
        }
        var it = ms.find(v[i]);
        ms.erase(it);
        {
          var j = cpp_char("0");
          while ((j <= cpp_char("9")))
          {
            v[i][0] = char(j);
            var t = true;
            {
              var z = 0;
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
    for (var u in v)
    {
      write(u, "\n");
    }
  }
  return 0;
}
