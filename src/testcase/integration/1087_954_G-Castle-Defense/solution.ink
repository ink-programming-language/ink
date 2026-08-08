// Translated from solution.cpp.

var dx = [0, 0, 1, -1, -1, -1, 1, 1];

var dy = [1, -1, 0, 0, -1, 1, 1, -1];

func biton(n: dynamic, pos: dynamic)
{
  return (n | ((cpp_cast(1) << pos)));
}

func bitoff(n: dynamic, pos: dynamic)
{
  return (n & (~((cpp_cast(1) << pos))));
}

func ison(n: dynamic, pos: dynamic)
{
  return cpp_cast(((n & ((cpp_cast(1) << pos)))));
}

func gcd(a: dynamic, b: dynamic)
{
  while (b)
  {
    a %= b;
    swap(a, b);
  }
  return a;
}

func NumberToString(Number: dynamic)
{
  var second: dynamic;
  (second << Number);
  return second.str();
}

func nxt()
{
  var aaa: dynamic;
  scanf("%d", (&aaa));
  return aaa;
}

func lxt()
{
  var aaa: dynamic;
  scanf("%lld", (&aaa));
  return aaa;
}

func dxt()
{
  var aaa: dynamic;
  scanf("%lf", (&aaa));
  return aaa;
}

func bigmod(p: dynamic, e: dynamic, m: dynamic)
{
  var ret = 1;
  {
    while ((e > 0))
    {
      if ((e & 1))
      {
        ret = (((ret * p)) % m);
      }
      p = (((p * p)) % m);
      e >>= 1;
    }
  }
  return cpp_cast(ret);
}

var ar = cpp_array(600010);

var sum = cpp_array(600010);

var cum = cpp_array(600010);

var tree = cpp_array(600010);

func update(pos: dynamic, limit: dynamic, val: dynamic)
{
  while ((pos <= limit))
  {
    tree[pos] += val;
    pos += (pos & ((-pos)));
  }
}

func query(pos: dynamic)
{
  var s = 0;
  while ((pos > 0))
  {
    s += tree[pos];
    pos -= (pos & ((-pos)));
  }
  return s;
}

func go(mid: dynamic, k: dynamic, n: dynamic, r: dynamic)
{
  var i = 1;
  memset(cum, 0, cpp_sizeof((cum)));
  while ((i <= n))
  {
    cum[i] += cum[(i - 1)];
    if (((sum[i] + cum[i]) < mid))
    {
      var extra = (mid - ((sum[i] + cum[i])));
      cum[i] += extra;
      cum[min(((i + (2 * r)) + 1), (n + 1))] -= extra;
      k -= extra;
      if ((k < 0))
      {
        return false;
      }
    }
    i += 1;
  }
  return (k >= 0);
}

func main()
{
  var n = nxt();
  var r = nxt();
  var k = lxt();
  {
    var i = 1;
    while ((i <= n))
    {
      ar[i] = lxt();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var le = (i - r);
      var ri = (i + r);
      le = max(le, 1);
      ri = min(ri, n);
      sum[le] += ar[i];
      sum[(ri + 1)] -= ar[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sum[i] += sum[(i - 1)];
      i += 1;
    }
  }
  var b = 0;
  var e = cpp_cast(LLONG_MAX);
  while ((b <= e))
  {
    var mid = (((b + e)) / 2);
    if (go(mid, k, n, r))
    {
      b = (mid + 1);
    } else
    {
      e = (mid - 1);
    }
  }
  write((b - 1), "\n");
  return 0;
}
