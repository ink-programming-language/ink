// Translated from solution.cpp.

func read(x: dynamic)
{
  x = 0;
  var c = getchar();
  var f = 1;
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  x *= f;
}

func umin(x: dynamic, y: dynamic)
{
  x = if ((x < y)) x else y;
}

func umax(x: dynamic, y: dynamic)
{
  x = if ((x > y)) x else y;
}

func R()
{
  var seed = 416;
  return cpp_comma(cpp_assign(seed, "^=", (seed >> 5)), cpp_comma(cpp_assign(seed, "^=", (seed << 17)), cpp_assign(seed, "^=", (seed >> 13))));
}

var N = 2666;

var n: dynamic;

var a = cpp_array(N);

var ans = cpp_array(1766666);

var tot: dynamic;

var sta = cpp_array(N);

var top: dynamic;

func move(i: dynamic, j: dynamic)
{
  ans[cpp_update(tot, "++")] = pair(i, j);
  assert((a[i] <= a[j]));
  a[j] -= a[i];
  a[i] += a[i];
}

func solve(x: dynamic, y: dynamic, z: dynamic)
{
  if ((a[y] > a[z]))
  {
    swap(y, z);
  }
  if ((a[x] > a[y]))
  {
    swap(x, y);
  }
  if ((a[y] > a[z]))
  {
    swap(y, z);
  }
  assert(((a[x] <= a[y]) && (a[y] <= a[z])));
  if ((!a[x]))
  {
    return;
  }
  if (((a[y] % a[x]) <= (a[x] / 2)))
  {
    var k = (a[y] / a[x]);
    var cur = 1;
    while (k)
    {
      if ((k & cur))
      {
        k ^= cur;
        move(x, y);
      } else
      {
        move(x, z);
      }
      cur <<= 1;
    }
  } else
  {
    var k = ((a[y] / a[x]) + 1);
    var mi = 1;
    while (((mi * 2) <= k))
    {
      mi *= 2;
    }
    var tmp = (k - mi);
    {
      var c = 1;
      while (((c * 2) <= mi))
      {
        if ((tmp & c))
        {
          tmp ^= c;
          move(x, y);
        } else
        {
          move(x, z);
        }
        c *= 2;
      }
    }
    move(y, x);
  }
  solve(x, y, z);
}

func main()
{
  read(n);
  {
    var i = (1);
    while ((i <= (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (n)))
    {
      if (a[i])
      {
        sta[cpp_update(top, "++")] = i;
      }
      i += 1;
    }
  }
  if ((top <= 1))
  {
    puts("-1");
    return 0;
  }
  while ((top >= 3))
  {
    var x = sta[cpp_update(top, "--")];
    var y = sta[cpp_update(top, "--")];
    var z = sta[cpp_update(top, "--")];
    solve(x, y, z);
    if (a[x])
    {
      sta[cpp_update(top, "++")] = x;
    }
    if (a[y])
    {
      sta[cpp_update(top, "++")] = y;
    }
    if (a[z])
    {
      sta[cpp_update(top, "++")] = z;
    }
  }
  printf("%d\n", tot);
  {
    var i = (1);
    while ((i <= (tot)))
    {
      printf("%d %d\n", ans[i].first, ans[i].second);
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = (1);
    while ((i <= (n)))
    {
      cnt += (a[i] > 0);
      i += 1;
    }
  }
  assert((cnt == 2));
  return 0;
}
