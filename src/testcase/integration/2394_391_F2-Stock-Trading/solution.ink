// Translated from solution.cpp.

func debug(a: dynamic, b: dynamic)
{
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func operator_shift_left(out: dynamic, a: dynamic)
{
  (((((out << cpp_char("(")) << a.first) << cpp_char(",")) << a.second) << cpp_char(")"));
  return out;
}

func readL()
{
  var res: dynamic;
  scanf("%I64d", (&res));
  return res;
}

func printL(res: dynamic)
{
  printf("%I64d", res);
}

var INF = 1e18;

var n: dynamic;

var k: dynamic;

var inter: dynamic;

var exist: dynamic;

var ar = cpp_array(100005);

func main()
{
  read(n, k);
  k += 2;
  var m = 0;
  ar[0] = INF;
  ar[1] = (2 * INF);
  {
    var i = 0;
    while ((i < (n)))
    {
      ar[(i + 2)] = readL();
      i += 1;
    }
  }
  ar[(n + 2)] = (-INF);
  ar[(n + 3)] = 0;
  n += 4;
  var res = 0;
  {
    var i = 0;
    while ((i < (n)))
    {
      var j = i;
      while ((((j + 1) < n) && (ar[(j + 1)] <= ar[j])))
      {
        j += 1;
      }
      var k = (j + 1);
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
    var i = 0;
    while ((i < (m)))
    {
      exist.insert(i);
      i += 1;
    }
  }
  n = m;
  {
    var i = 0;
    while ((i < ((m / 2))))
    {
      inter.insert(make_pair((ar[((i * 2) + 1)] - ar[(i * 2)]), make_pair((i * 2), 0)));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (((m / 2) - 1))))
    {
      inter.insert(make_pair((ar[((i * 2) + 1)] - ar[((i * 2) + 2)]), make_pair(((i * 2) + 1), 1)));
      i += 1;
    }
  }
  {
    var hoge = 0;
    while ((hoge < (((m / 2) - k))))
    {
      var it = inter.begin();
      res -= it->first;
      if ((it->second.second == 0))
      {
        var pos = it->second.first;
        var p1 = exist.find(pos);
        var p2 = p1;
        p2 += 1;
        var q2 = p1;
        q2 -= 1;
        var q1 = q2;
        q1 -= 1;
        var r1 = p2;
        r1 += 1;
        var r2 = r1;
        r2 += 1;
        inter.erase(inter.begin());
        inter.erase(make_pair((-((ar[(*p1)] - ar[(*q2)]))), make_pair((*q2), 1)));
        inter.erase(make_pair((-((ar[(*r1)] - ar[(*p2)]))), make_pair((*p2), 1)));
        inter.insert(make_pair((-((ar[(*r1)] - ar[(*q2)]))), make_pair((*q2), 1)));
        exist.erase(p2);
        exist.erase(pos);
      } else
      {
        var pos = it->second.first;
        var p2 = exist.find(pos);
        var p1 = p2;
        p1 -= 1;
        inter.erase(inter.begin());
        inter.erase(make_pair((ar[(*p2)] - ar[(*p1)]), make_pair((*p1), 0)));
        var q1 = p2;
        q1 += 1;
        var q2 = q1;
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
