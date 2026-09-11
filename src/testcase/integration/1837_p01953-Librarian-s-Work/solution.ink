// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func repn(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

var pb: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var sc: dynamic = cpp_expression("#inclu");

var mp: dynamic = cpp_expression("#include");

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var za: dynamic = cpp_array(((1 << 18)));

var sum: dynamic = cpp_array(((1 << 18)));

var num: dynamic = cpp_array(((1 << 18)));

var a: dynamic = cpp_array(100005);

var cc: dynamic = cpp_array(100005);

func add(i: dynamic, v: dynamic) -> dynamic
{
  i += (((1 << 17)) - 1);
  za[i].pb(v);
}

func make() -> dynamic
{
  {
    var i: dynamic = (((1 << 18)) - 1);
    while ((i >= (((1 << 17)) - 1)))
    {
      sort(za[i].begin(), za[i].end());
      za[i].erase(unique(za[i].begin(), za[i].end()), za[i].end());
      sum[i].resize((za[i].size() + 1), 0);
      num[i].resize((za[i].size() + 1), 0);
      i -= 1;
    }
  }
  {
    var i: dynamic = (((1 << 17)) - 2);
    while ((i >= 0))
    {
      za[i].resize((za[((i * 2) + 1)].size() + za[((i * 2) + 2)].size()));
      merge(za[((i * 2) + 1)].begin(), za[((i * 2) + 1)].end(), za[((i * 2) + 2)].begin(), za[((i * 2) + 2)].end(), za[i].begin());
      za[i].erase(unique(za[i].begin(), za[i].end()), za[i].end());
      sum[i].resize((za[i].size() + 1), 0);
      num[i].resize((za[i].size() + 1), 0);
      i -= 1;
    }
  }
}

func f(x: dynamic) -> dynamic
{
  return (x & (-x));
}

func addsum(pos: dynamic, k: dynamic, a: dynamic) -> dynamic
{
  {
    var i: dynamic = k;
    while ((i < sum[pos].size()))
    {
      sum[pos][i] += a;
      i += f(i);
    }
  }
}

func addnum(pos: dynamic, k: dynamic, a: dynamic) -> dynamic
{
  {
    var i: dynamic = k;
    while ((i < num[pos].size()))
    {
      num[pos][i] += a;
      i += f(i);
    }
  }
}

func sumsum(pos: dynamic, k: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    var i: dynamic = k;
    while ((i > 0))
    {
      res += sum[pos][i];
      i -= f(i);
    }
  }
  return res;
}

func sumnum(pos: dynamic, k: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    var i: dynamic = k;
    while ((i > 0))
    {
      res += num[pos][i];
      i -= f(i);
    }
  }
  return res;
}

func query(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic, aa: dynamic) -> dynamic
{
  if (((r < a) || (b < l)))
  {
    return 0;
  }
  if (((a <= l) && (r <= b)))
  {
    var x: dynamic = (lower_bound(za[k].begin(), za[k].end(), aa) - za[k].begin());
    var v: dynamic = (((sumsum(k, x) * 1) * c) + ((sumnum(k, x) * 1) * aa));
    v += (((sumsum(k, (sum[k].size() - 1)) - sumsum(k, x))) + (((((sumnum(k, (sum[k].size() - 1)) - sumnum(k, x))) * 1) * c) * aa));
    return v;
  }
  return (query(a, b, ((k * 2) + 1), l, (((l + r)) / 2), aa) + query(a, b, ((k * 2) + 2), ((((l + r)) / 2) + 1), r, aa));
}

func make2(a: dynamic, b: dynamic) -> dynamic
{
  a += (((1 << 17)) - 1);
  var c: dynamic = ((lower_bound(za[a].begin(), za[a].end(), b) - za[a].begin()) + 1);
  addsum(a, c, b);
  addnum(a, c, 1);
  while (a)
  {
    a = (((a - 1)) / 2);
    c = ((lower_bound(za[a].begin(), za[a].end(), b) - za[a].begin()) + 1);
    addsum(a, c, b);
    addnum(a, c, 1);
  }
}

func main() -> dynamic
{
  read(n, c);
  rep(i, n);
  read(a[i], cc[i]);
  make();
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ret += query((a[i] + 1), (((1 << 17)) - 1), 0, 0, (((1 << 17)) - 1), cc[i]);
      make2(a[i], cc[i]);
      i += 1;
    }
  }
  write(ret, "\n");
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    add(a[i], cc[i]);
  }
