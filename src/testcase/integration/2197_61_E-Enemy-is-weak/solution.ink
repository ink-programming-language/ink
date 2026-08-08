// Translated from solution.cpp.

var Mod = (7 + 1e9);

var INF32 = (5 + 2e9);

var INF64 = (5 + 1e18);

var MAX = (5 + 1e6);

var n: dynamic;

var a = cpp_array(MAX);

var b = cpp_array(MAX);

var T1 = cpp_array((4 * MAX));

var T2 = cpp_array((4 * MAX));

var TAR: dynamic;

var VAL: dynamic;

func update(T: dynamic, x: dynamic = 1, l: dynamic = 0, r: dynamic = (n - 1))
{
  if (((l > TAR) || (r < TAR)))
  {
    return T[x];
  }
  if ((l == r))
  {
    return cpp_assign(T[x], "=", VAL);
  }
  var mid = ((l + r) >> 1);
  var c1 = (x << 1);
  var c2 = ((x << 1) | 1);
  return cpp_assign(T[x], "=", (update(T, c1, l, mid) + update(T, c2, (mid + 1), r)));
}

func query(T: dynamic, x: dynamic = 1, l: dynamic = 0, r: dynamic = (n - 1))
{
  if ((r < TAR))
  {
    return 0;
  }
  if ((l >= TAR))
  {
    return T[x];
  }
  var mid = ((l + r) >> 1);
  var c1 = (x << 1);
  var c2 = ((x << 1) | 1);
  return (query(T, c1, l, mid) + query(T, c2, (mid + 1), r));
}

func main()
{
  read(n);
  {
    var i = 0;
    var j: dynamic;
    while ((i < n))
    {
      scanf("%d", (&j));
      b[i] = pair(j, i);
      i += 1;
    }
  }
  sort(b, (b + n));
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      TAR = (b[i].second + 1);
      if ((TAR < (n - 1)))
      {
        ans += query(T2);
      }
      VAL = query(T1);
      TAR = b[i].second;
      update(T2);
      VAL = 1;
      update(T1);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
