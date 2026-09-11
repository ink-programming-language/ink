// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v1: dynamic = cpp_uninitialized();

var v2: dynamic = cpp_uninitialized();

var a1: dynamic = cpp_uninitialized();

var a2: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var first: dynamic = cpp_uninitialized();
      var second: dynamic = cpp_uninitialized();
      read(first, second);
      v1.push_back([min(first, second), max(first, second)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var first: dynamic = cpp_uninitialized();
      var second: dynamic = cpp_uninitialized();
      read(first, second);
      v2.push_back([min(first, second), max(first, second)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var s1: dynamic = cpp_uninitialized();
      {
        var k: dynamic = 0;
        while ((k < m))
        {
          if ((v1[i] == v2[k]))
          {
            k += 1;
            continue;
          }
          var s: dynamic = cpp_uninitialized();
          s.insert(v1[i].first);
          s.insert(v1[i].second);
          s.insert(v2[k].first);
          s.insert(v2[k].second);
          if ((s.count(v1[i].first) == 2))
          {
            s1.insert(v1[i].first);
          }
          if ((s.count(v1[i].second) == 2))
          {
            s1.insert(v1[i].second);
          }
          k += 1;
        }
      }
      if ((cpp_cast(s1.size()) > 1))
      {
        write(-1, cpp_char("\n"));
        return;
      }
      if ((cpp_cast(s1.size()) == 1))
      {
        a1.insert((*s1.begin()));
      }
      i += 1;
    }
  }
  swap(v1, v2);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var s1: dynamic = cpp_uninitialized();
      {
        var k: dynamic = 0;
        while ((k < n))
        {
          if ((v1[i] == v2[k]))
          {
            k += 1;
            continue;
          }
          var s: dynamic = cpp_uninitialized();
          s.insert(v1[i].first);
          s.insert(v1[i].second);
          s.insert(v2[k].first);
          s.insert(v2[k].second);
          if ((s.count(v1[i].first) == 2))
          {
            s1.insert(v1[i].first);
          }
          if ((s.count(v1[i].second) == 2))
          {
            s1.insert(v1[i].second);
          }
          k += 1;
        }
      }
      if ((cpp_cast(s1.size()) > 1))
      {
        write(-1, cpp_char("\n"));
        return;
      }
      if ((cpp_cast(s1.size()) == 1))
      {
        a2.insert((*s1.begin()));
      }
      i += 1;
    }
  }
  if (cpp_binary((cpp_cast(a1.size()) == 1), "and", (cpp_cast(a2.size()) == 1)))
  {
    write((*a1.begin()), cpp_char("\n"));
  } else
  {
    write(0, cpp_char("\n"));
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
