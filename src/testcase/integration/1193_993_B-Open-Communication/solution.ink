// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var v1: dynamic;

var v2: dynamic;

var a1: dynamic;

var a2: dynamic;

func solve()
{
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      var first: dynamic;
      var second: dynamic;
      read(first, second);
      v1.push_back([min(first, second), max(first, second)]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var first: dynamic;
      var second: dynamic;
      read(first, second);
      v2.push_back([min(first, second), max(first, second)]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var s1: dynamic;
      {
        var k = 0;
        while ((k < m))
        {
          if ((v1[i] == v2[k]))
          {
            k += 1;
            continue;
          }
          var s: dynamic;
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
    var i = 0;
    while ((i < m))
    {
      var s1: dynamic;
      {
        var k = 0;
        while ((k < n))
        {
          if ((v1[i] == v2[k]))
          {
            k += 1;
            continue;
          }
          var s: dynamic;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
