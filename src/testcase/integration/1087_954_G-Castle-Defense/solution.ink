// Translated from solution.cpp.

var dx: dynamic = [0, 0, 1, -1, -1, -1, 1, 1];

var dy: dynamic = [1, -1, 0, 0, -1, 1, 1, -1];

func biton(n: dynamic, pos: dynamic) -> dynamic
{
  return (n | ((cpp_cast(1) << pos)));
}

func bitoff(n: dynamic, pos: dynamic) -> dynamic
{
  return (n & (~((cpp_cast(1) << pos))));
}

func ison(n: dynamic, pos: dynamic) -> dynamic
{
  return cpp_cast(((n & ((cpp_cast(1) << pos)))));
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    a %= b;
    swap(a, b);
  }
  return a;
}

func NumberToString(Number: dynamic) -> dynamic
{
  var second: dynamic = cpp_uninitialized();
  (second << Number);
  return second.str();
}

func nxt() -> dynamic
{
  var aaa: dynamic = cpp_uninitialized();
  scanf("%d", (&aaa));
  return aaa;
}

func lxt() -> dynamic
{
  var aaa: dynamic = cpp_uninitialized();
  scanf("%lld", (&aaa));
  return aaa;
}

func dxt() -> dynamic
{
  var aaa: dynamic = cpp_uninitialized();
  scanf("%lf", (&aaa));
  return aaa;
}

func bigmod(p: dynamic, e: dynamic, m: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

var ar: dynamic = cpp_array(600010);

var sum: dynamic = cpp_array(600010);

var cum: dynamic = cpp_array(600010);

var tree: dynamic = cpp_array(600010);

func update(pos: dynamic, limit: dynamic, val: dynamic) -> dynamic
{
  while ((pos <= limit))
  {
    tree[pos] += val;
    pos += (pos & ((-pos)));
  }
}

func query(pos: dynamic) -> dynamic
{
  var s: dynamic = 0;
  while ((pos > 0))
  {
    s += tree[pos];
    pos -= (pos & ((-pos)));
  }
  return s;
}

func go(mid: dynamic, k: dynamic, n: dynamic, r: dynamic) -> dynamic
{
  var i: dynamic = 1;
  memset(cum, 0, cpp_sizeof((cum)));
  while ((i <= n))
  {
    cum[i] += cum[(i - 1)];
    if (((sum[i] + cum[i]) < mid))
    {
      var extra: dynamic = (mid - ((sum[i] + cum[i])));
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

func main() -> dynamic
{
  var n: dynamic = nxt();
  var r: dynamic = nxt();
  var k: dynamic = lxt();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ar[i] = lxt();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var le: dynamic = (i - r);
      var ri: dynamic = (i + r);
      le = max(le, 1);
      ri = min(ri, n);
      sum[le] += ar[i];
      sum[(ri + 1)] -= ar[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sum[i] += sum[(i - 1)];
      i += 1;
    }
  }
  var b: dynamic = 0;
  var e: dynamic = cpp_cast(LLONG_MAX);
  while ((b <= e))
  {
    var mid: dynamic = (((b + e)) / 2);
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
