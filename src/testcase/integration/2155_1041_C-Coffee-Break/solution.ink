// Translated from solution.cpp.

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      ((os << v[i]) << " ");
      i += 1;
    }
  }
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  for (var it: dynamic in v)
  {
    ((os << it) << " ");
  }
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (((os << v.first) << " ") << v.second);
  return os;
}

var mod: dynamic = (1e9 + 7);

var inf: dynamic = 2e18;

var ninf: dynamic = -2e18;

func pow(a: dynamic, b: dynamic, m: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      ans = (((ans * a)) % m);
    }
    b /= 2;
    a = (((a * a)) % m);
  }
  return ans;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  t1 = clock();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, m, d);
  var arr: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_construct(n, -1);
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      s.insert(make_pair(x, i));
      i += 1;
    }
  }
  var day: dynamic = 1;
  while ((!s.empty()))
  {
    var curr: dynamic = s.begin()->first;
    var cind: dynamic = s.begin()->second;
    s.erase(s.begin());
    ans[cind] = day;
    while (1)
    {
      var it: dynamic = s.lower_bound(make_pair(((curr + d) + 1), ninf));
      if ((it == s.end()))
      {
        day += 1;
        break;
      }
      ans[it->second] = day;
      curr = it->first;
      s.erase(it);
    }
  }
  write((day - 1), cpp_char("\n"));
  write(ans, cpp_char("\n"));
  t2 = clock();
  write(cpp_char("\n"), (t2 - t1), cpp_char("\n"));
  return 0;
}
