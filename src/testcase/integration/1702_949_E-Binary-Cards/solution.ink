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

func gcd1(x: dynamic, y: dynamic)
{
  var z = y;
  while (((x % y) != 0))
  {
    z = (x % y);
    x = y;
    y = z;
  }
  return z;
}

var fg: dynamic;

var tmp: dynamic;

var ans: dynamic;

var C: dynamic;

var stp: dynamic;

func cal(v: dynamic, dep: dynamic)
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
  var vp: dynamic;
  var vp1: dynamic;
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
  for (var c in v)
  {
    if (((c % 2) == 0))
    {
      var tar = (c / 2);
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
  var OS = stp;
  stp += 1;
  var N = vp.size();
  for (var z in vp1)
  {
    var tar = (((z + 1)) / 2);
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
  for (var z in vp1)
  {
    var tar = (((z - 1)) / 2);
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

func fmain(ID: dynamic)
{
  scanf("%d", (&n));
  var vp: dynamic;
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
  for (var z in ans)
  {
    printf("%d ", z);
  }
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
