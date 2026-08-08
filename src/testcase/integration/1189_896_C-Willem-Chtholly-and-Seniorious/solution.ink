// Translated from solution.cpp.

var INF = (1e18 + 7);

var N = (1e5 + 7);

var MOD = (1e9 + 7);

var n: dynamic;

var m: dynamic;

var seed: dynamic;

var v_max: dynamic;

var i: dynamic;

var j: dynamic;

var k: dynamic;

var a = cpp_array(N);

var op: dynamic;

var l: dynamic;

var r: dynamic;

var x: dynamic;

var y: dynamic;

func plusi(a: dynamic, b: dynamic, mo: dynamic)
{
  var cur = ((((0 + a) + b)) % mo);
  return cur;
}

func multi(a: dynamic, b: dynamic, mo: dynamic)
{
  a %= mo;
  b %= mo;
  var cur = ((((1 * a) * b)) % mo);
  return cur;
}

func power(base: dynamic, pow: dynamic, mo: dynamic)
{
  if ((pow == 0))
  {
    return 1;
  }
  if ((pow == 1))
  {
    base %= mo;
    return base;
  }
  var cur = power(base, (pow / 2), mo);
  cur = multi(cur, cur, mo);
  if ((pow % 2))
  {
    cur = multi(cur, base, mo);
  }
  return cur;
}

func rnd()
{
  var cur = seed;
  seed = (((((seed * 7)) + 13)) % MOD);
  return cur;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n, m, seed, v_max);
  {
    i = 1;
    while ((i <= n))
    {
      a[i] = ((rnd() % v_max) + 1);
      i += 1;
    }
  }
  var prv: dynamic;
  var pq: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      var u = [i, i];
      pq.push(u);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      op = ((rnd() % 4) + 1);
      l = ((rnd() % n) + 1);
      r = ((rnd() % n) + 1);
      if ((l > r))
      {
        swap(l, r);
      }
      if ((op == 3))
      {
        x = ((rnd() % (((r - l) + 1))) + 1);
      } else
      {
        x = ((rnd() % v_max) + 1);
      }
      if ((op == 4))
      {
        y = ((rnd() % v_max) + 1);
      }
      if ((op == 1))
      {
        while ((!pq.empty()))
        {
          var cur = pq.front();
          pq.pop();
          var q = cur.first;
          var w = cur.second;
          if (((q == l) && (w == r)))
          {
            a[q] += x;
          }
          if (((q == l) && (w < r)))
          {
            a[q] += x;
          }
          if (((q == l) && (w > r)))
          {
            prv.push([q, r]);
            a[(r + 1)] = a[q];
            a[q] += x;
            q = (r + 1);
          }
          if (((q > l) && (w == r)))
          {
            a[q] += x;
          }
          if ((((q > l) && (w > r)) && (q <= r)))
          {
            prv.push([q, r]);
            a[(r + 1)] = a[q];
            a[q] += x;
            q = (r + 1);
          }
          if (((q < l) && (w == r)))
          {
            prv.push([q, (l - 1)]);
            a[l] = (a[q] + x);
            q = l;
          }
          if ((((q < l) && (w < r)) && (w >= l)))
          {
            prv.push([q, (l - 1)]);
            a[l] = (a[q] + x);
            q = l;
          }
          if (((q > l) && (w < r)))
          {
            a[q] += x;
          }
          if (((q < l) && (w > r)))
          {
            prv.push([q, (l - 1)]);
            prv.push([(r + 1), w]);
            a[(r + 1)] = a[q];
            a[l] = (a[q] + x);
            q = l;
            w = r;
          }
          prv.push([q, w]);
        }
      }
      if ((op == 2))
      {
        while ((!pq.empty()))
        {
          var cur = pq.front();
          pq.pop();
          var q = cur.first;
          var w = cur.second;
          if (((q == l) && (w > r)))
          {
            prv.push([(r + 1), w]);
            a[(r + 1)] = a[q];
          }
          if (((q < l) && (w == r)))
          {
            prv.push([q, (l - 1)]);
          }
          if (((q < l) && (w > r)))
          {
            prv.push([q, (l - 1)]);
            prv.push([(r + 1), w]);
            a[(r + 1)] = a[q];
          }
          if ((((q < l) && (w < r)) && (w >= l)))
          {
            prv.push([q, (l - 1)]);
          }
          if ((((q > l) && (w > r)) && (q <= r)))
          {
            prv.push([(r + 1), w]);
            a[(r + 1)] = a[q];
          }
          if (((w < l) || (q > r)))
          {
            prv.push([q, w]);
          }
        }
        prv.push([l, r]);
        a[l] = x;
      }
      if ((op == 3))
      {
        var ans: dynamic;
        while ((!pq.empty()))
        {
          var cur = pq.front();
          pq.pop();
          var q = cur.first;
          var w = cur.second;
          if (((q == l) && (w == r)))
          {
            ans.push([(-a[q]), ((r - l) + 1)]);
          }
          if (((q == l) && (w < r)))
          {
            ans.push([(-a[q]), ((w - l) + 1)]);
          }
          if (((q == l) && (w > r)))
          {
            ans.push([(-a[q]), ((r - l) + 1)]);
          }
          if (((q > l) && (w == r)))
          {
            ans.push([(-a[q]), ((r - q) + 1)]);
          }
          if (((q < l) && (w == r)))
          {
            ans.push([(-a[q]), ((r - l) + 1)]);
          }
          if ((((q > l) && (w > r)) && (q <= r)))
          {
            ans.push([(-a[q]), ((r - q) + 1)]);
          }
          if (((q > l) && (w < r)))
          {
            ans.push([(-a[q]), ((w - q) + 1)]);
          }
          if ((((q < l) && (w < r)) && (w >= l)))
          {
            ans.push([(-a[q]), ((w - l) + 1)]);
          }
          if (((q < l) && (w > r)))
          {
            ans.push([(-a[q]), ((r - l) + 1)]);
          }
          prv.push([q, w]);
        }
        var here = 0;
        var prin = (1e18 + 7);
        while ((here < x))
        {
          here += ans.top().second;
          prin = (-ans.top().first);
          ans.pop();
        }
        write(prin, "\n");
      }
      if ((op == 4))
      {
        var ans = 0;
        while ((!pq.empty()))
        {
          var cur = pq.front();
          pq.pop();
          var q = cur.first;
          var w = cur.second;
          if (((q == l) && (w == r)))
          {
            ans = multi(((r - l) + 1), power(a[q], x, y), y);
          }
          if (((q == l) && (w < r)))
          {
            ans = plusi(ans, multi(((w - q) + 1), power(a[q], x, y), y), y);
          }
          if (((q == l) && (w > r)))
          {
            ans = multi(((r - l) + 1), power(a[q], x, y), y);
          }
          if (((q > l) && (w == r)))
          {
            ans = plusi(ans, multi(((w - q) + 1), power(a[q], x, y), y), y);
          }
          if (((q < l) && (w == r)))
          {
            ans = multi(((r - l) + 1), power(a[q], x, y), y);
          }
          if ((((q < l) && (w < r)) && (w >= l)))
          {
            ans = plusi(ans, multi(((w - l) + 1), power(a[q], x, y), y), y);
          }
          if (((q > l) && (w < r)))
          {
            ans = plusi(ans, multi(((w - q) + 1), power(a[q], x, y), y), y);
          }
          if ((((q > l) && (w > r)) && (q <= r)))
          {
            ans = plusi(ans, multi(((r - q) + 1), power(a[q], x, y), y), y);
          }
          if (((q < l) && (w > r)))
          {
            ans = multi(((r - l) + 1), power(a[q], x, y), y);
          }
          prv.push([q, w]);
        }
        write(ans, "\n");
      }
      while ((!prv.empty()))
      {
        pq.push(prv.front());
        prv.pop();
      }
      i += 1;
    }
  }
  return 0;
}
