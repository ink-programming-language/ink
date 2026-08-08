// Translated from solution.cpp.

var debug = 0;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

var direc = "RDLU";

var ln: dynamic;

var lk: dynamic;

var lm: dynamic;

func etp(f: dynamic = 0)
{
  puts(if (f) "YES" else "NO");
  exit(0);
}

func addmod(x: dynamic, y: dynamic, mod: dynamic = 1000000007)
{
  assert((y >= 0));
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
  assert(((x >= 0) && (x < mod)));
}

func et()
{
  puts("-1");
  exit(0);
}

func fastPow(x: dynamic, y: dynamic, mod: dynamic = 1000000007)
{
  var ans = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      ans = (((x * ans)) % mod);
    }
    x = ((x * x) % mod);
    y >>= 1;
  }
  return ans;
}

var a = cpp_array(400105);

var s = cpp_array(400105);

var t = cpp_array(400105);

var st = cpp_array(400105);

var C: dynamic;

var ans: dynamic;

var T = cpp_array((400105 * 4));

var lz = cpp_array((400105 * 4));

func Up(rt: dynamic, l: dynamic, r: dynamic)
{
  T[rt] = max(T[((rt << 1))], T[(((rt << 1) | 1))]);
}

func Down(rt: dynamic, l: dynamic, r: dynamic)
{
  if ((lz[rt] != 0))
  {
    T[((rt << 1))] += lz[rt];
    lz[((rt << 1))] += lz[rt];
    T[(((rt << 1) | 1))] += lz[rt];
    lz[(((rt << 1) | 1))] += lz[rt];
    lz[rt] = 0;
  }
}

func upt(rt: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic, val: dynamic)
{
  if (((R <= l) || (r <= L)))
  {
    return;
  }
  if (((L <= l) && (r <= R)))
  {
    T[rt] += val;
    lz[rt] += val;
    return;
  }
  Down(rt, l, r);
  var mid = (((l + r)) / 2);
  upt(((rt << 1)), l, mid, L, R, val);
  upt((((rt << 1) | 1)), mid, r, L, R, val);
  Up(rt, l, r);
}

func uptS(rt: dynamic, l: dynamic, r: dynamic, L: dynamic, val: dynamic)
{
  if (((l + 1) == r))
  {
    T[rt] = val;
    return;
  }
  Down(rt, l, r);
  var mid = (((l + r)) / 2);
  if ((L <= mid))
  {
    uptS(((rt << 1)), l, mid, L, val);
  } else
  {
    uptS((((rt << 1) | 1)), mid, r, L, val);
  }
  Up(rt, l, r);
}

func qy(rt: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic)
{
  if (((R <= l) || (r <= L)))
  {
    return 0;
  }
  if (((L <= l) && (r <= R)))
  {
    return T[rt];
  }
  Down(rt, l, r);
  var mid = (((l + r)) / 2);
  return max(qy(((rt << 1)), l, mid, L, R), qy((((rt << 1) | 1)), mid, r, L, R));
}

func fmain(tid: dynamic)
{
  scanf("%d%lld", (&n), (&C));
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((((n + n) - 1)))))
    {
      scanf("%lld", (a + i));
      (i) += 1;
    }
  }
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((((n + n) - 1)))))
    {
      s[i] = (s[(i - 1)] + a[i]);
      (i) += 1;
    }
  }
  var i = 0;
  {
    int_cpp(j) = 1;
    while (((j) <= cpp_cast((n))))
    {
      while ((((i + 1) <= j) && ((s[((j + j) - 1)] - s[(((((i + 1)) * 2) - 1) - 1)]) >= C)))
      {
        i += 1;
      }
      if (i)
      {
        var z = max((C - ((st[(j - 1)] - st[(i - 1)]))), 0);
        t[j] = max(t[j], z);
      }
      if (((j > 1) && (i < (j - 1))))
      {
        upt(1, 0, n, i, (j - 1), ((a[((j * 2) - 1)] + a[((j * 2) - 2)]) - t[(j - 1)]));
      }
      uptS(1, 0, n, j, a[((j + j) - 1)]);
      var z = qy(1, 0, n, i, j);
      t[j] = max(t[j], z);
      ans += t[j];
      st[j] = (st[(j - 1)] + t[j]);
      (j) += 1;
    }
  }
  printf("%lld\n", ans);
}

func main()
{
  var t = 1;
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((t))))
    {
      fmain(i);
      (i) += 1;
    }
  }
  return 0;
}
