// Translated from solution.cpp.

func chmax(first: dynamic, second: dynamic) -> dynamic
{
  if ((first < second))
  {
    first = second;
  }
}

func chmin(first: dynamic, second: dynamic) -> dynamic
{
  if ((second < first))
  {
    first = second;
  }
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << "{") << p.first) << ",") << p.second) << "}");
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << "{");
  for (var e: dynamic in v)
  {
    ((os << e) << ",");
  }
  return (os << "}");
}

func operator_shift_left(os: dynamic, first: dynamic) -> dynamic
{
  return (os << vc(first.begin(), first.end()));
}

func print_tuple(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
}

func print_tuple(os: dynamic, t: dynamic) -> dynamic
{
  if (i)
  {
    (os << ",");
  }
  (os << get(t));
  print_tuple(os, t);
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  (os << "{");
  print_tuple(os, t);
  return (os << "}");
}

func print(x: dynamic, suc: dynamic = 1) -> dynamic
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

func read() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  read(i);
  return i;
}

func readvi(n: dynamic, off: dynamic = 0) -> dynamic
{
  {
    var i: dynamic = ll(0);
    while ((i < ll(n)))
    {
      v[i] = (read() + off);
      i += 1;
    }
  }
  return v;
}

func print(v: dynamic, suc: dynamic = 1) -> dynamic
{
  {
    var i: dynamic = ll(0);
    while ((i < ll(v.size())))
    {
      print(v[i],  ((i == (ll(v.size()) - 1))) ? suc : 2);
      i += 1;
    }
  }
}

func readString() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  return s;
}

func sq(t: dynamic) -> dynamic
{
  return (t * t);
}

func yes(ex: dynamic = true) -> dynamic
{
  write("Yes", "\n");
  if (ex)
  {
    exit(0);
  }
}

func no(ex: dynamic = true) -> dynamic
{
  write("No", "\n");
  if (ex)
  {
    exit(0);
  }
}

func possible(ex: dynamic = true) -> dynamic
{
  write("Possible", "\n");
  if (ex)
  {
    exit(0);
  }
}

func impossible(ex: dynamic = true) -> dynamic
{
  write("Impossible", "\n");
  if (ex)
  {
    exit(0);
  }
}

func ten(n: dynamic) -> dynamic
{
  return  ((n == 0)) ? 1 : (ten((n - 1)) * 10);
}

var infLL: dynamic = (LLONG_MAX / 3);

var inf: dynamic = infLL;

func topbit(t: dynamic) -> dynamic
{
  return  ((t == 0)) ? -1 : (31 - builtin_clz(t));
}

func topbit(t: dynamic) -> dynamic
{
  return  ((t == 0)) ? -1 : (63 - builtin_clzll(t));
}

func botbit(first: dynamic) -> dynamic
{
  return  ((first == 0)) ? 32 : builtin_ctz(first);
}

func botbit(first: dynamic) -> dynamic
{
  return  ((first == 0)) ? 64 : builtin_ctzll(first);
}

func popcount(t: dynamic) -> dynamic
{
  return builtin_popcount(t);
}

func popcount(t: dynamic) -> dynamic
{
  return builtin_popcountll(t);
}

func ispow2(i: dynamic) -> dynamic
{
  return (i && (((i & (-i))) == i));
}

func mask(i: dynamic) -> dynamic
{
  return (((ll(1) << i)) - 1);
}

func inc(first: dynamic, second: dynamic, c: dynamic) -> dynamic
{
  return ((first <= second) && (second <= c));
}

func mkuni(v: dynamic) -> dynamic
{
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
}

func rand_int(l: dynamic, r: dynamic) -> dynamic
{
  var gen: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());
  return uniform_int_distribution(l, r)(gen);
}

func myshuffle(first: dynamic) -> dynamic
{
  {
    var i: dynamic = ll(0);
    while ((i < ll(ll(first.size()))))
    {
      swap(first[i], first[rand_int(0, i)]);
      i += 1;
    }
  }
}

func lwb(v: dynamic, first: dynamic) -> dynamic
{
  return (lower_bound(v.begin(), v.end(), first) - v.begin());
}

class unionfind
{
  var p: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  func unionfind(n: dynamic) -> dynamic
  {
      self->p = cpp_construct(n, -1);
      self->s = cpp_construct(n, 1);
      self->c = cpp_construct(n);
    }
  func find(first: dynamic) -> dynamic
  {
      return  ((p[first] == -1)) ? first : (cpp_assign(p[first], "=", find(p[first])));
    }
  func unite(first: dynamic, second: dynamic) -> dynamic
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
  func same(first: dynamic, second: dynamic) -> dynamic
  {
      return (find(first) == find(second));
    }
  func sz(first: dynamic) -> dynamic
  {
      return s[find(first)];
    }
}

func slv() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var uf: dynamic = cpp_construct((n + 1));
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = ll(0);
    while ((i < ll(n)))
    {
      s.insert(1);
      i += 1;
    }
  }
  var mg: dynamic = __cpp_lambda_1;
  {
    var i: dynamic = ll(0);
    while ((i < ll(n)))
    {
      var p: dynamic = cpp_uninitialized();
      read(p);
      p -= 1;
      qs[p] = i;
      i += 1;
    }
  }
  for (var i: dynamic in qs)
  {
    var mx: dynamic = (*s.rbegin());
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

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  write(fixed, setprecision(20));
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var cpp_name: dynamic = ll(0);
    while ((cpp_name < ll(t)))
    {
      slv();
      cpp_name += 1;
    }
  }
}

func __cpp_lambda_1(i: dynamic) -> dynamic
{
  assert((uf.find(i) == i));
  s.erase(s.find(uf.sz(i)));
  var j: dynamic = uf.find((i + 1));
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
