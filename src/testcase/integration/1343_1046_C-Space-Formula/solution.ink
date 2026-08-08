// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  d -= 1;
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(vals[i]);
      i += 1;
    }
  }
  var chk = (vals[0] + v[d]);
  var idx = 0;
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if (((i == d) || ((v[i] > chk))))
      {
        ans += 1;
        idx += 1;
        i += 1;
        continue;
      } else
      {
        s.insert(v[i]);
      }
      i += 1;
    }
  }
  {
    while ((idx < n))
    {
      if (((s.size() + idx) != n))
      {
        write("Da la beng ba");
        return 0;
      }
      var it = s.lower_bound(((chk - vals[idx]) + 1));
      if ((it == s.begin()))
      {
        ans += 1;
        s.erase(cpp_update(s.end(), "--"));
      } else
      {
        it -= 1;
        s.erase(it);
      }
      idx += 1;
    }
  }
  write(ans);
  return 0;
}
