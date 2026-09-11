// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var mod: dynamic = 1000000007;

var linf: dynamic = 0x3f3f3f3f3f3f3f3f;

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func lis(v: dynamic) -> dynamic
{
  var ans: dynamic = cpp_construct(v.size());
  var color: dynamic = 0;
  var best: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      var it: dynamic = best.lower_bound([v[i], 0]);
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

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(s.size())))
    {
      v.push_back(s[i]);
      i += 1;
    }
  }
  var aux: dynamic = lis(v);
  var color: dynamic = aux.first;
  var ans: dynamic = aux.second;
  write(color, "\n");
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func main() -> dynamic
{
  cin.tie(0)->sync_with_stdio(0);
  solve();
  return 0;
}
