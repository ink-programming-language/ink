// Translated from solution.cpp.

var s: dynamic = cpp_array(50005);

var n: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(3005, 3005);

var dpc: dynamic = cpp_array(3005, 3005);

func gogo() -> dynamic
{
  var c: dynamic = cpp_array(26);
  memset((c), (0), cpp_sizeof(((c))));
  {
    int_cpp(i) = cpp_cast((1));
    while (((i) <= cpp_cast((n))))
    {
      c[(s[i] - cpp_char("a"))] += 1;
      (i) += 1;
    }
  }
  var z: dynamic = -1;
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast((26))))
    {
      if ((c[i] > 100))
      {
        z = i;
      }
      (i) += 1;
    }
  }
  assert((~z));
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast((100))))
    {
      putchar((cpp_char("a") + z));
      (i) += 1;
    }
  }
  return 0;
}

func gao(l: dynamic, r: dynamic) -> dynamic
{
  var res: dynamic = dp[l][r];
  if ((~res))
  {
    return res;
  }
  if ((l == r))
  {
    return cpp_comma(cpp_assign(dpc[l][r], "=", 3), cpp_assign(res, "=", 1));
  }
  if ((l > r))
  {
    return cpp_comma(cpp_assign(dpc[l][r], "=", 4), cpp_assign(res, "=", 0));
  }
  res = 0;
  var t: dynamic = cpp_uninitialized();
  if ((s[l] == s[r]))
  {
    t = (gao((l + 1), (r - 1)) + 2);
    if ((t > res))
    {
      res = t;
      dpc[l][r] = 3;
    }
  }
  t = gao((l + 1), r);
  if ((t > res))
  {
    res = t;
    dpc[l][r] = 1;
  }
  t = gao(l, (r - 1));
  if ((t > res))
  {
    res = t;
    dpc[l][r] = 2;
  }
  return res;
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  scanf("%s", (s + 1));
  n = strlen((s + 1));
  if ((n > 2600))
  {
    return gogo();
  }
  memset((dp), (-1), cpp_sizeof(((dp))));
  var max_len: dynamic = gao(1, n);
  var ans: dynamic = cpp_uninitialized();
  var l: dynamic = 1;
  var r: dynamic = n;
  var lst: dynamic = 0;
  while ((l <= r))
  {
    if ((dpc[l][r] == 3))
    {
      if ((l == r))
      {
        lst = 1;
      }
      ans += s[l];
      l += 1;
      r -= 1;
    } else if ((dpc[l][r] == 2))
    {
      r -= 1;
    } else if ((dpc[l][r] == 1))
    {
      l += 1;
    } else
    {
      assert(0);
    }
  }
  var h: dynamic = cpp_cast((ans).size());
  {
    var i: dynamic = (h - ((lst + 1)));
    while ((i >= 0))
    {
      ans += ans[i];
      i -= 1;
    }
  }
  var m: dynamic = cpp_cast((ans).size());
  assert((m == max_len));
  if ((m < 100))
  {
    return (puts(ans.c_str()) & 0);
  }
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast((50))))
    {
      putchar(ans[i]);
      (i) += 1;
    }
  }
  {
    int_cpp(i) = cpp_cast((49));
    while (((i) >= cpp_cast((0))))
    {
      putchar(ans[i]);
      (i) -= 1;
    }
  }
  puts("");
  return 0;
}
