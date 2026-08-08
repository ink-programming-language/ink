// Translated from solution.cpp.

var N = 100005;

var v = cpp_array(5);

var w = cpp_array(5);

var id = cpp_array(5);

var p1 = cpp_array(N);

var p2 = cpp_array(N);

func gcd(x: dynamic, y: dynamic)
{
  return if (y) gcd(y, (x % y)) else x;
}

var sum: dynamic;

func insert(x: dynamic, fl: dynamic)
{
  if (((!p1[x]) && (!p2[x])))
  {
    if (fl) cpp_update(sum, "++") else cpp_update(sum, "--");
  }
}

func check()
{
  return (sum != 0);
}

var mp: dynamic;

var sum: dynamic;

func insert(x: dynamic, fl: dynamic)
{
  if ((fl == 1))
  {
    if ((!mp[pair(p1[x], p2[x])]))
    {
      if (mp[pair((-p1[x]), (-p2[x]))])
      {
        sum += 1;
      }
    }
    mp[pair(p1[x], p2[x])] += 1;
  } else
  {
    mp[pair(p1[x], p2[x])] -= 1;
    if ((!mp[pair(p1[x], p2[x])]))
    {
      if (mp[pair((-p1[x]), (-p2[x]))])
      {
        sum -= 1;
      }
    }
  }
}

func check()
{
  return (sum != 0);
}

var mn = cpp_array((N * 4));

var mx = cpp_array((N * 4));

var fl = cpp_array((N * 4));

func cmp(x: dynamic, y: dynamic)
{
  var v = (((1.0 * p1[x]) * p2[y]) - ((1.0 * p1[y]) * p2[x]));
  if ((fabs(v) > 1e12))
  {
    return if ((v > 0)) 1 else -1;
  }
  var vv = ((p1[x] * p2[y]) - (p1[y] * p2[x]));
  return (if (vv) (if ((vv > 0)) 1 else -1) else 0);
}

func pushup(k: dynamic)
{
  var ls = (k * 2);
  var rs = ((k * 2) + 1);
  fl[k] = (fl[ls] | fl[rs]);
  if (((!mn[ls]) || (!mn[rs])))
  {
    mn[k] = (mn[ls] + mn[rs]);
    mx[k] = (mx[ls] + mx[rs]);
  } else
  {
    if (((cmp(mn[ls], mn[rs]) == -1) && (cmp(mn[ls], mx[rs]) == 1)))
    {
      fl[k] = 1;
    }
    if (((cmp(mn[rs], mn[ls]) == -1) && (cmp(mn[rs], mx[ls]) == 1)))
    {
      fl[k] = 1;
    }
    if (((cmp(mx[ls], mn[rs]) == -1) && (cmp(mx[ls], mx[rs]) == 1)))
    {
      fl[k] = 1;
    }
    if (((cmp(mx[rs], mn[ls]) == -1) && (cmp(mx[rs], mx[ls]) == 1)))
    {
      fl[k] = 1;
    }
    mn[k] = (if ((cmp(mn[ls], mn[rs]) == -1)) mn[ls] else mn[rs]);
    mx[k] = (if ((cmp(mx[ls], mx[rs]) == -1)) mx[rs] else mx[ls]);
  }
}

func insert(k: dynamic, l: dynamic, r: dynamic, x: dynamic, v: dynamic)
{
  if ((l == r))
  {
    mn[k] = cpp_assign(mx[k], "=", (v * x));
    fl[k] = 0;
    return;
  }
  var mid = (((l + r)) / 2);
  if ((x <= mid))
  {
    insert((k * 2), l, mid, x, v);
  } else
  {
    insert(((k * 2) + 1), (mid + 1), r, x, v);
  }
  pushup(k);
}

func insert(x: dynamic, v: dynamic)
{
  insert(1, 1, (N - 1), x, v);
}

func check()
{
  return fl[1];
}

func main()
{
  scanf("%d%d%d", (&v[1]), (&v[2]), (&v[3]));
  id[1] = 1;
  id[2] = 2;
  id[3] = 3;
  if ((!v[id[1]]))
  {
    swap(id[1], id[2]);
  }
  if ((!v[id[1]]))
  {
    swap(id[1], id[3]);
  }
  var Q: dynamic;
  var n = 0;
  scanf("%d", (&Q));
  while (cpp_update(Q, "--"))
  {
    var s = cpp_array(10);
    scanf("%s", (s + 1));
    if ((s[1] == cpp_char("A")))
    {
      scanf("%d%d%d", (&w[1]), (&w[2]), (&w[3]));
      n += 1;
      p1[n] = (((1 * w[id[2]]) * v[id[1]]) - ((1 * v[id[2]]) * w[id[1]]));
      p2[n] = (((1 * w[id[3]]) * v[id[1]]) - ((1 * v[id[3]]) * w[id[1]]));
      var G = gcd(abs(p1[n]), abs(p2[n]));
      if (G)
      {
        p1[n] /= G;
        p2[n] /= G;
      }
      N1.insert(n, 1);
      N2.insert(n, 1);
      N3.insert(n, 1);
    } else
    {
      var x: dynamic;
      scanf("%d", (&x));
      N1.insert(x, 0);
      N2.insert(x, 0);
      N3.insert(x, 0);
    }
    if (N1.check())
    {
      puts("1");
    } else if (N2.check())
    {
      puts("2");
    } else if (N3.check())
    {
      puts("3");
    } else
    {
      puts("0");
    }
  }
}
