// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func repn(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

var pb = cpp_expression("#include");

var fi = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var mp = cpp_expression("#include");

var n: dynamic;

var c: dynamic;

var za = cpp_array(((1 << 18)));

var sum = cpp_array(((1 << 18)));

var num = cpp_array(((1 << 18)));

var a = cpp_array(100005);

var cc = cpp_array(100005);

func add(i: dynamic, v: dynamic)
{
  i += (((1 << 17)) - 1);
  za[i].pb(v);
}

func make()
{
  {
    var i = (((1 << 18)) - 1);
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
    var i = (((1 << 17)) - 2);
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

func f(x: dynamic)
{
  return (x & (-x));
}

func addsum(pos: dynamic, k: dynamic, a: dynamic)
{
  {
    var i = k;
    while ((i < sum[pos].size()))
    {
      sum[pos][i] += a;
      i += f(i);
    }
  }
}

func addnum(pos: dynamic, k: dynamic, a: dynamic)
{
  {
    var i = k;
    while ((i < num[pos].size()))
    {
      num[pos][i] += a;
      i += f(i);
    }
  }
}

func sumsum(pos: dynamic, k: dynamic)
{
  var res = 0;
  {
    var i = k;
    while ((i > 0))
    {
      res += sum[pos][i];
      i -= f(i);
    }
  }
  return res;
}

func sumnum(pos: dynamic, k: dynamic)
{
  var res = 0;
  {
    var i = k;
    while ((i > 0))
    {
      res += num[pos][i];
      i -= f(i);
    }
  }
  return res;
}

func query(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic, aa: dynamic)
{
  if (((r < a) || (b < l)))
  {
    return 0;
  }
  if (((a <= l) && (r <= b)))
  {
    var x = (lower_bound(za[k].begin(), za[k].end(), aa) - za[k].begin());
    var v = (((sumsum(k, x) * 1) * c) + ((sumnum(k, x) * 1) * aa));
    v += (((sumsum(k, (sum[k].size() - 1)) - sumsum(k, x))) + (((((sumnum(k, (sum[k].size() - 1)) - sumnum(k, x))) * 1) * c) * aa));
    return v;
  }
  return (query(a, b, ((k * 2) + 1), l, (((l + r)) / 2), aa) + query(a, b, ((k * 2) + 2), ((((l + r)) / 2) + 1), r, aa));
}

func make2(a: dynamic, b: dynamic)
{
  a += (((1 << 17)) - 1);
  var c = ((lower_bound(za[a].begin(), za[a].end(), b) - za[a].begin()) + 1);
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

func main()
{
  read(n, c);
  rep(i, n);
  read(a[i], cc[i]);
  make();
  var ret = 0;
  {
    var i = 0;
    while ((i < n))
    {
      ret += query((a[i] + 1), (((1 << 17)) - 1), 0, 0, (((1 << 17)) - 1), cc[i]);
      make2(a[i], cc[i]);
      i += 1;
    }
  }
  write(ret, "\n");
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    add(a[i], cc[i]);
  }
