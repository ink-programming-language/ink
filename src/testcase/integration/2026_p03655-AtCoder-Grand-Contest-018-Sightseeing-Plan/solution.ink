// Translated from solution.cpp.

var mod = cpp_expression("#include<b");

var reg = cpp_expression("#include");

var maxn = cpp_expression("#includ");

var inv = cpp_array(maxn);

var fac = cpp_array(maxn);

var ifac = cpp_array(maxn);

var X1: dynamic;

var X2: dynamic;

var X3: dynamic;

var X4: dynamic;

var X5: dynamic;

var X6: dynamic;

var Y1: dynamic;

var Y2: dynamic;

var Y3: dynamic;

var Y4: dynamic;

var Y5: dynamic;

var Y6: dynamic;

var f = cpp_array(maxn);

var g = cpp_array(maxn);

var ans: dynamic;

func inc(x: dynamic, y: dynamic)
{
  return if (((x + y) >= mod)) ((x + y) - mod) else (x + y);
}

func C(x: dynamic, y: dynamic)
{
  return (((((1 * fac[x]) * ifac[y]) % mod) * ifac[(x - y)]) % mod);
}

func get_dis(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  var lenx = abs((x2 - x1));
  var leny = abs((y2 - y1));
  return C((lenx + leny), lenx);
}

func get2(x: dynamic, y: dynamic, l: dynamic, r: dynamic, d: dynamic, u: dynamic)
{
  var res = 0;
  res = inc(get_dis(x, y, (r + 1), (u + 1)), res);
  res = inc((mod - get_dis(x, y, l, (u + 1))), res);
  res = inc((mod - get_dis(x, y, (r + 1), d)), res);
  res = inc(get_dis(x, y, l, d), res);
  return res;
}

func get1(x: dynamic, y: dynamic, l: dynamic, r: dynamic, d: dynamic, u: dynamic)
{
  var res = 0;
  res = inc(res, get_dis(x, y, r, u));
  res = inc(res, (mod - get_dis(x, y, (l - 1), u)));
  res = inc(res, (mod - get_dis(x, y, r, (d - 1))));
  res = inc(res, get_dis(x, y, (l - 1), (d - 1)));
  return res;
}

func main()
{
  read(X1, X2, X3, X4, X5, X6);
  read(Y1, Y2, Y3, Y4, Y5, Y6);
  inv[0] = 1;
  ifac[0] = 1;
  fac[0] = 1;
  {
    var i = 1;
    while ((i < maxn))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      inv[i] = if (((i == 1))) 1 else (((1 * inv[(mod % i)]) * ((mod - (mod / i)))) % mod);
      ifac[i] = (((1 * ifac[(i - 1)]) * inv[i]) % mod);
      i += 1;
    }
  }
  {
    var i = Y3;
    while ((i <= Y4))
    {
      f[i] = get1(X4, i, X1, X2, Y1, Y2);
      f[i] = (((1 * f[i]) * get2((X4 + 1), i, X5, X6, Y5, Y6)) % mod);
      f[i] = (((1 * f[i]) * (((X4 + i) + 1))) % mod);
      ans = inc(ans, f[i]);
      i += 1;
    }
  }
  {
    var i = X3;
    while ((i <= X4))
    {
      g[i] = get1(i, Y4, X1, X2, Y1, Y2);
      g[i] = (((1 * g[i]) * get2(i, (Y4 + 1), X5, X6, Y5, Y6)) % mod);
      g[i] = (((1 * g[i]) * (((i + Y4) + 1))) % mod);
      ans = inc(ans, g[i]);
      i += 1;
    }
  }
  {
    var i = Y3;
    while ((i <= Y4))
    {
      f[i] = get1((X3 - 1), i, X1, X2, Y1, Y2);
      f[i] = (((1 * f[i]) * get2(X3, i, X5, X6, Y5, Y6)) % mod);
      f[i] = (((1 * f[i]) * (((mod - X3) - i))) % mod);
      ans = inc(ans, f[i]);
      i += 1;
    }
  }
  {
    var i = X3;
    while ((i <= X4))
    {
      g[i] = get1(i, (Y3 - 1), X1, X2, Y1, Y2);
      g[i] = (((1 * g[i]) * get2(i, Y3, X5, X6, Y5, Y6)) % mod);
      g[i] = (((1 * g[i]) * (((mod - Y3) - i))) % mod);
      ans = inc(ans, g[i]);
      i += 1;
    }
  }
  write(ans, "\n");
}
