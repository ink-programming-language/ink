// Translated from solution.cpp.

func operator_shift_left(os: dynamic, v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      ((os << v[i]) << " ");
      i += 1;
    }
  }
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  for (var it in v)
  {
    ((os << it) << " ");
  }
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (((os << v.first) << " ") << v.second);
  return os;
}

var mod = (1e9 + 7);

var inf = 2e18;

var ninf = -2e18;

func pow(a: dynamic, b: dynamic, m: dynamic)
{
  var ans = 1;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t1: dynamic;
  var t2: dynamic;
  t1 = clock();
  var n: dynamic;
  var m: dynamic;
  var d: dynamic;
  read(n, m, d);
  var arr: dynamic;
  var ans = cpp_construct(n, -1);
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      s.insert(make_pair(x, i));
      i += 1;
    }
  }
  var day = 1;
  while ((!s.empty()))
  {
    var curr = s.begin()->first;
    var cind = s.begin()->second;
    s.erase(s.begin());
    ans[cind] = day;
    while (1)
    {
      var it = s.lower_bound(make_pair(((curr + d) + 1), ninf));
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
