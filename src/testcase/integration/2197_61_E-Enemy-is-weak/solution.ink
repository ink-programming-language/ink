// Translated from solution.cpp.

var Mod: dynamic = (7 + 1e9);

var INF32: dynamic = (5 + 2e9);

var INF64: dynamic = (5 + 1e18);

var MAX: dynamic = (5 + 1e6);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAX);

var b: dynamic = cpp_array(MAX);

var T1: dynamic = cpp_array((4 * MAX));

var T2: dynamic = cpp_array((4 * MAX));

var TAR: dynamic = cpp_uninitialized();

var VAL: dynamic = cpp_uninitialized();

func update(T: dynamic, x: dynamic = 1, l: dynamic = 0, r: dynamic = (n - 1)) -> dynamic
{
  if (((l > TAR) || (r < TAR)))
  {
    return T[x];
  }
  if ((l == r))
  {
    return cpp_assign(T[x], "=", VAL);
  }
  var mid: dynamic = ((l + r) >> 1);
  var c1: dynamic = (x << 1);
  var c2: dynamic = ((x << 1) | 1);
  return cpp_assign(T[x], "=", (update(T, c1, l, mid) + update(T, c2, (mid + 1), r)));
}

func query(T: dynamic, x: dynamic = 1, l: dynamic = 0, r: dynamic = (n - 1)) -> dynamic
{
  if ((r < TAR))
  {
    return 0;
  }
  if ((l >= TAR))
  {
    return T[x];
  }
  var mid: dynamic = ((l + r) >> 1);
  var c1: dynamic = (x << 1);
  var c2: dynamic = ((x << 1) | 1);
  return (query(T, c1, l, mid) + query(T, c2, (mid + 1), r));
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    var j: dynamic = cpp_uninitialized();
    while ((i < n))
    {
      scanf("%d", (&j));
      b[i] = pair(j, i);
      i += 1;
    }
  }
  sort(b, (b + n));
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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
