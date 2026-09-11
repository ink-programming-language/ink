// Translated from solution.cpp.

var INF: dynamic = 1e9;

var L: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  var res: dynamic = 0;
  while ((L.size() > 1))
  {
    var mini: dynamic = INF;
    {
      var it: dynamic = L.begin();
      while ((it != L.end()))
      {
        var x: dynamic = it->second;
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
      var it: dynamic = L.begin();
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
      var it: dynamic = L.begin();
      while ((it != L.end()))
      {
        var cur: dynamic = cpp_update(it, "++");
        if ((cur->second <= 0))
        {
          L.erase(cur);
          continue;
        }
        if ((cur != L.begin()))
        {
          var pre: dynamic = prev(cur);
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var last: dynamic = s[0];
  var cnt: dynamic = 0;
  s.push_back(cpp_char("$"));
  {
    var i: dynamic = 0;
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
