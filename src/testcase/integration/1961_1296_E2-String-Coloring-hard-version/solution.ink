// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var mod = 1000000007;

var linf = 0x3f3f3f3f3f3f3f3f;

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func lis(v: dynamic)
{
  var ans = cpp_construct(v.size());
  var color = 0;
  var best: dynamic;
  {
    var i = 0;
    while ((i < v.size()))
    {
      var it = best.lower_bound([v[i], 0]);
      if (((it == best.begin()) && (((*it)).first != v[i])))
      {
        color += 1;
        ans[i] = color;
        best.insert([v[i], i]);
      } else if ((((*it)).first != v[i]))
      {
        it -= 1;
        ans[i] = ans[((*it)).second];
        best.insert([v[i], i]);
        best.erase(it);
      } else
      {
        ans[i] = ans[((*it)).second];
        best.insert([v[i], i]);
        best.erase(it);
      }
      i += 1;
    }
  }
  return [color, ans];
}

func solve()
{
  var n: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var v: dynamic;
  {
    var i = 0;
    while ((i < cpp_cast(s.size())))
    {
      v.push_back(s[i]);
      i += 1;
    }
  }
  var aux = lis(v);
  var color = aux.first;
  var ans = aux.second;
  write(color, "\n");
  {
    var i = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func main()
{
  cin.tie(0)->sync_with_stdio(0);
  solve();
  return 0;
}
