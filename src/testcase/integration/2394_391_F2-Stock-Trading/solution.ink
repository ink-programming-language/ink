// Translated from solution.cpp.

func debug(a: dynamic, b: dynamic) -> dynamic
{
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func operator_shift_left(out: dynamic, a: dynamic) -> dynamic
{
  (((((out << cpp_char("(")) << a.first) << cpp_char(",")) << a.second) << cpp_char(")"));
  return out;
}

func readL() -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  scanf("%I64d", (&res));
  return res;
}

func printL(res: dynamic) -> dynamic
{
  printf("%I64d", res);
}

var INF: dynamic = 1e18;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var inter: dynamic = cpp_uninitialized();

var exist: dynamic = cpp_uninitialized();

var ar: dynamic = cpp_array(100005);

func main() -> dynamic
{
  read(n, k);
  k += 2;
  var m: dynamic = 0;
  ar[0] = INF;
  ar[1] = (2 * INF);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      ar[(i + 2)] = readL();
      i += 1;
    }
  }
  ar[(n + 2)] = (-INF);
  ar[(n + 3)] = 0;
  n += 4;
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      var j: dynamic = i;
      while ((((j + 1) < n) && (ar[(j + 1)] <= ar[j])))
      {
        j += 1;
      }
      var k: dynamic = (j + 1);
      while ((((k + 1) < n) && (ar[(k + 1)] >= ar[k])))
      {
        k += 1;
      }
      if ((k < n))
      {
        res += (ar[k] - ar[j]);
        ar[cpp_update(m, "++")] = ar[j];
        ar[cpp_update(m, "++")] = ar[k];
      }
      i = k;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      exist.insert(i);
      i += 1;
    }
  }
  n = m;
  {
    var i: dynamic = 0;
    while ((i < ((m / 2))))
    {
      inter.insert(make_pair((ar[((i * 2) + 1)] - ar[(i * 2)]), make_pair((i * 2), 0)));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (((m / 2) - 1))))
    {
      inter.insert(make_pair((ar[((i * 2) + 1)] - ar[((i * 2) + 2)]), make_pair(((i * 2) + 1), 1)));
      i += 1;
    }
  }
  {
    var hoge: dynamic = 0;
    while ((hoge < (((m / 2) - k))))
    {
      var it: dynamic = inter.begin();
      res -= it->first;
      if ((it->second.second == 0))
      {
        var pos: dynamic = it->second.first;
        var p1: dynamic = exist.find(pos);
        var p2: dynamic = p1;
        p2 += 1;
        var q2: dynamic = p1;
        q2 -= 1;
        var q1: dynamic = q2;
        q1 -= 1;
        var r1: dynamic = p2;
        r1 += 1;
        var r2: dynamic = r1;
        r2 += 1;
        inter.erase(inter.begin());
        inter.erase(make_pair((-((ar[(*p1)] - ar[(*q2)]))), make_pair((*q2), 1)));
        inter.erase(make_pair((-((ar[(*r1)] - ar[(*p2)]))), make_pair((*p2), 1)));
        inter.insert(make_pair((-((ar[(*r1)] - ar[(*q2)]))), make_pair((*q2), 1)));
        exist.erase(p2);
        exist.erase(pos);
      } else
      {
        var pos: dynamic = it->second.first;
        var p2: dynamic = exist.find(pos);
        var p1: dynamic = p2;
        p1 -= 1;
        inter.erase(inter.begin());
        inter.erase(make_pair((ar[(*p2)] - ar[(*p1)]), make_pair((*p1), 0)));
        var q1: dynamic = p2;
        q1 += 1;
        var q2: dynamic = q1;
        q2 += 1;
        inter.erase(make_pair((ar[(*q2)] - ar[(*q1)]), make_pair((*q1), 0)));
        inter.insert(make_pair((ar[(*q2)] - ar[(*p1)]), make_pair((*p1), 0)));
        exist.erase(p2);
        q1 = exist.lower_bound(pos);
        exist.erase(q1);
      }
      hoge += 1;
    }
  }
  res -= (2 * INF);
  write(res, "\n");
  return 0;
}
