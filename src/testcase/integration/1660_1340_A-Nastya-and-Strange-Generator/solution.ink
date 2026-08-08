// Translated from solution.cpp.

func chmax(first: dynamic, second: dynamic)
{
  if ((first < second))
  {
    first = second;
  }
}

func chmin(first: dynamic, second: dynamic)
{
  if ((second < first))
  {
    first = second;
  }
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << "{") << p.first) << ",") << p.second) << "}");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "{");
  for (var e in v)
  {
    ((os << e) << ",");
  }
  return (os << "}");
}

func operator_shift_left(os: dynamic, first: dynamic)
{
  return (os << vc(first.begin(), first.end()));
}

func print_tuple(argument_0: dynamic, argument_1: dynamic)
{
}

func print_tuple(os: dynamic, t: dynamic)
{
  if (i)
  {
    (os << ",");
  }
  (os << get(t));
  print_tuple(os, t);
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "{");
  print_tuple(os, t);
  return (os << "}");
}

func print(x: dynamic, suc: dynamic = 1)
{
  write(x);
  if ((suc == 1))
  {
    write("\n");
  }
  if ((suc == 2))
  {
    write(" ");
  }
}

func read()
{
  var i: dynamic;
  read(i);
  return i;
}

func readvi(n: dynamic, off: dynamic = 0)
{
  {
    var i = ll(0);
    while ((i < ll(n)))
    {
      v[i] = (read() + off);
      i += 1;
    }
  }
  return v;
}

func print(v: dynamic, suc: dynamic = 1)
{
  {
    var i = ll(0);
    while ((i < ll(v.size())))
    {
      print(v[i], if ((i == (ll(v.size()) - 1))) suc else 2);
      i += 1;
    }
  }
}

func readString()
{
  var s: dynamic;
  read(s);
  return s;
}

func sq(t: dynamic)
{
  return (t * t);
}

func yes(ex: dynamic = true)
{
  write("Yes", "\n");
  if (ex)
  {
    exit(0);
  }
}

func no(ex: dynamic = true)
{
  write("No", "\n");
  if (ex)
  {
    exit(0);
  }
}

func possible(ex: dynamic = true)
{
  write("Possible", "\n");
  if (ex)
  {
    exit(0);
  }
}

func impossible(ex: dynamic = true)
{
  write("Impossible", "\n");
  if (ex)
  {
    exit(0);
  }
}

func ten(n: dynamic)
{
  return if ((n == 0)) 1 else (ten((n - 1)) * 10);
}

var infLL = (LLONG_MAX / 3);

var inf = infLL;

func topbit(t: dynamic)
{
  return if ((t == 0)) -1 else (31 - builtin_clz(t));
}

func topbit(t: dynamic)
{
  return if ((t == 0)) -1 else (63 - builtin_clzll(t));
}

func botbit(first: dynamic)
{
  return if ((first == 0)) 32 else builtin_ctz(first);
}

func botbit(first: dynamic)
{
  return if ((first == 0)) 64 else builtin_ctzll(first);
}

func popcount(t: dynamic)
{
  return builtin_popcount(t);
}

func popcount(t: dynamic)
{
  return builtin_popcountll(t);
}

func ispow2(i: dynamic)
{
  return (i && (((i & (-i))) == i));
}

func mask(i: dynamic)
{
  return (((ll(1) << i)) - 1);
}

func inc(first: dynamic, second: dynamic, c: dynamic)
{
  return ((first <= second) && (second <= c));
}

func mkuni(v: dynamic)
{
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
}

func rand_int(l: dynamic, r: dynamic)
{
  var gen = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());
  return uniform_int_distribution(l, r)(gen);
}

func myshuffle(first: dynamic)
{
  {
    var i = ll(0);
    while ((i < ll(ll(first.size()))))
    {
      swap(first[i], first[rand_int(0, i)]);
      i += 1;
    }
  }
}

func lwb(v: dynamic, first: dynamic)
{
  return (lower_bound(v.begin(), v.end(), first) - v.begin());
}

class unionfind
{
  var p: dynamic;
  var s: dynamic;
  var c: dynamic;
  func unionfind(n: dynamic)
  {
      this->p = cpp_construct(n, -1);
      this->s = cpp_construct(n, 1);
      this->c = cpp_construct(n);
    }
  func find(first: dynamic)
  {
      return if ((p[first] == -1)) first else (cpp_assign(p[first], "=", find(p[first])));
    }
  func unite(first: dynamic, second: dynamic)
  {
      first = find(first);
      second = find(second);
      if ((first == second))
      {
        return false;
      }
      p[second] = first;
      s[first] += s[second];
      c -= 1;
      return true;
    }
  func same(first: dynamic, second: dynamic)
  {
      return (find(first) == find(second));
    }
  func sz(first: dynamic)
  {
      return s[find(first)];
    }
}

func slv()
{
  var n: dynamic;
  read(n);
  var uf = cpp_construct((n + 1));
  var s: dynamic;
  {
    var i = ll(0);
    while ((i < ll(n)))
    {
      s.insert(1);
      i += 1;
    }
  }
  var mg = __cpp_lambda_1;
  {
    var i = ll(0);
    while ((i < ll(n)))
    {
      var p: dynamic;
      read(p);
      p -= 1;
      qs[p] = i;
      i += 1;
    }
  }
  for (var i in qs)
  {
    var mx = (*s.rbegin());
    if ((uf.sz(i) == mx))
    {
      mg(i);
    } else
    {
      no(0);
      return;
    }
  }
  yes(0);
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  write(fixed, setprecision(20));
  var t: dynamic;
  read(t);
  {
    var cpp_name = ll(0);
    while ((cpp_name < ll(t)))
    {
      slv();
      cpp_name += 1;
    }
  }
}

func __cpp_lambda_1(i: dynamic)
{
  assert((uf.find(i) == i));
  s.erase(s.find(uf.sz(i)));
  var j = uf.find((i + 1));
  if ((j < n))
  {
    s.erase(s.find(uf.sz(j)));
  }
  uf.unite(j, i);
  if ((j < n))
  {
    s.insert(uf.sz(j));
  }
}
