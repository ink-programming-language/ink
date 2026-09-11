// Translated from solution.cpp.

var debug: dynamic = 0;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [1, 0, -1, 0];

var direc: dynamic = "RDLU";

var ln: dynamic = cpp_uninitialized();

var lk: dynamic = cpp_uninitialized();

var lm: dynamic = cpp_uninitialized();

func etp(f: dynamic = 0) -> dynamic
{
  puts( (f) ? "YES" : "NO");
  exit(0);
}

func addmod(x: dynamic, y: dynamic, mod: dynamic = 1000000007) -> dynamic
{
  assert((y >= 0));
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
  assert(((x >= 0) && (x < mod)));
}

func et() -> dynamic
{
  puts("-1");
  exit(0);
}

func fastPow(x: dynamic, y: dynamic, mod: dynamic = 1000000007) -> dynamic
{
  var ans: dynamic = 1;
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

func gcd1(x: dynamic, y: dynamic) -> dynamic
{
  var z: dynamic = y;
  while (((x % y) != 0))
  {
    z = (x % y);
    x = y;
    y = z;
  }
  return z;
}

var fg: dynamic = cpp_uninitialized();

var tmp: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var stp: dynamic = cpp_uninitialized();

func cal(v: dynamic, dep: dynamic) -> dynamic
{
  if ((dep > 22))
  {
    return;
  }
  if (v.empty())
  {
    if (((!fg) || (tmp.size() < ans.size())))
    {
      ans = tmp;
      fg = 1;
    }
    return;
  }
  var vp: dynamic = cpp_uninitialized();
  var vp1: dynamic = cpp_uninitialized();
  if (((v.size() == 1) && (abs(v[0]) == 1)))
  {
    if ((v[0] == -1))
    {
      tmp.push_back((-((1 << dep))));
    } else
    {
      tmp.push_back((1 << dep));
    }
    cal(vp, (dep + 1));
    tmp.pop_back();
    return;
  } else if (((v.size() == 2) && (abs((cpp_cast(v[0]) * v[1])) == 1)))
  {
    if ((v[0] != v[1]))
    {
      tmp.push_back((-((1 << dep))));
      tmp.push_back((1 << dep));
    } else if ((v[0] == 1))
    {
      tmp.push_back((1 << dep));
    } else
    {
      tmp.push_back((-((1 << dep))));
    }
    cal(vp, (dep + 1));
    tmp.pop_back();
    if ((v[0] != v[1]))
    {
      tmp.pop_back();
    }
    return;
  }
  stp += 1;
  for (var c: dynamic in v)
  {
    if (((c % 2) == 0))
    {
      var tar: dynamic = (c / 2);
      if (((tar != 0) && (C[tar] != stp)))
      {
        C[tar] = stp;
        vp.push_back(tar);
      }
    } else
    {
      vp1.push_back(c);
    }
  }
  if (vp1.empty())
  {
    cal(vp, (dep + 1));
    return;
  }
  var OS: dynamic = stp;
  stp += 1;
  var N: dynamic = vp.size();
  for (var z: dynamic in vp1)
  {
    var tar: dynamic = (((z + 1)) / 2);
    if ((tar == 0))
    {
      continue;
    }
    if (((C[tar] == stp) || (C[tar] == OS)))
    {
      continue;
    }
    C[tar] = stp;
    vp.push_back(tar);
  }
  tmp.push_back((-((1 << dep))));
  cal(vp, (dep + 1));
  tmp.pop_back();
  while ((vp.size() > N))
  {
    vp.pop_back();
  }
  stp += 1;
  for (var z: dynamic in vp1)
  {
    var tar: dynamic = (((z - 1)) / 2);
    if ((tar == 0))
    {
      continue;
    }
    if (((C[tar] == stp) || (C[tar] == OS)))
    {
      continue;
    }
    C[tar] = stp;
    vp.push_back(tar);
  }
  tmp.push_back((1 << dep));
  cal(vp, (dep + 1));
  tmp.pop_back();
}

func fmain(ID: dynamic) -> dynamic
{
  scanf("%d", (&n));
  var vp: dynamic = cpp_uninitialized();
  stp += 1;
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((n))))
    {
      scanf("%d", (&k));
      if (((C[k] != stp) && (k != 0)))
      {
        C[k] = stp;
        vp.push_back(k);
      }
      (i) += 1;
    }
  }
  cal(vp, 0);
  printf("%d\n", ans.size());
  for (var z: dynamic in ans)
  {
    printf("%d ", z);
  }
}

func main() -> dynamic
{
  var t: dynamic = 1;
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
