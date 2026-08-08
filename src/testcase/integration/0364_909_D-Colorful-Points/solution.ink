// Translated from solution.cpp.

var INF = 1e9;

var L: dynamic;

func solve()
{
  var res = 0;
  while ((L.size() > 1))
  {
    var mini = INF;
    {
      var it = L.begin();
      while ((it != L.end()))
      {
        var x = it->second;
        if (((it == L.begin()) || (next(it) == L.end())))
        {
          mini = min(mini, x);
        } else
        {
          mini = min(mini, (((x + 1)) / 2));
        }
        it += 1;
      }
    }
    res += mini;
    {
      var it = L.begin();
      while ((it != L.end()))
      {
        if (((it == L.begin()) || (next(it) == L.end())))
        {
          it->second -= mini;
        } else
        {
          it->second -= (2 * mini);
        }
        it += 1;
      }
    }
    {
      var it = L.begin();
      while ((it != L.end()))
      {
        var cur = cpp_update(it, "++");
        if ((cur->second <= 0))
        {
          L.erase(cur);
          continue;
        }
        if ((cur != L.begin()))
        {
          var pre = prev(cur);
          if ((pre->first == cur->first))
          {
            pre->second += cur->second;
            L.erase(cur);
          }
        }
      }
    }
  }
  return res;
}

func main()
{
  ios_base.sync_with_stdio(false);
  var s: dynamic;
  read(s);
  var last = s[0];
  var cnt = 0;
  s.push_back(cpp_char("$"));
  {
    var i = 0;
    while ((i < cpp_cast(s.size())))
    {
      if ((s[i] == last))
      {
        cnt += 1;
      } else
      {
        L.emplace_back(last, cnt);
        last = s[i];
        cnt = 1;
      }
      i += 1;
    }
  }
  {
  }
  write(solve(), "\n");
}
