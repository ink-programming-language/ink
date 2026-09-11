// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d);
  d -= 1;
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(vals[i]);
      i += 1;
    }
  }
  var chk: dynamic = (vals[0] + v[d]);
  var idx: dynamic = 0;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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
      var it: dynamic = s.lower_bound(((chk - vals[idx]) + 1));
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
