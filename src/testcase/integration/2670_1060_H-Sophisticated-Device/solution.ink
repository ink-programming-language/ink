// Translated from solution.cpp.

var inf: dynamic = (1e9 + 7);

var INF: dynamic = (1e18 + 7);

func operator_shift_left(p: dynamic, x: dynamic) -> dynamic
{
  return (((((p << "<") << x.first) << ", ") << x.second) << ">");
}

func operator_shift_left(p: dynamic, y: dynamic) -> dynamic
{
  var o: dynamic = 0;
  (p << "{");
  for (var c: dynamic in y)
  {
    if (cpp_update(o, "++"))
    {
      (p << ", ");
    }
    (p << c);
  }
  return (p << "}");
}

func dor() -> dynamic
{
  write(cpp_char("\n"));
}

func dor(p: dynamic, y: dynamic...) -> dynamic
{
  write(p, " ");
  dor(cpp_expand(y));
}

func mini(p: dynamic, y: dynamic) -> dynamic
{
  if ((p > y))
  {
    p = y;
  }
}

func maxi(p: dynamic, y: dynamic) -> dynamic
{
  if ((p < y))
  {
    p = y;
  }
}

var d: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var curr: dynamic = cpp_uninitialized();

var gdzie: dynamic = cpp_array((1 << 10));

var t: dynamic = cpp_array(5007);

func fpow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      res = ((res * a) % p);
    }
    a = ((a * a) % p);
    b /= 2;
  }
  return res;
}

func add(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  write("+ ", a, " ", b, " ", c, cpp_char("\n"));
}

func raise(a: dynamic, b: dynamic) -> dynamic
{
  write("^ ", a, " ", b, cpp_char("\n"));
}

func mult(ind: dynamic, chce: dynamic) -> dynamic
{
  if ((chce == 1))
  {
    return;
  }
  add(ind, gdzie[0], 4999);
  mult(ind, (chce / 2));
  add(ind, ind, ind);
  if ((chce % 2))
  {
    add(ind, 4999, ind);
  }
}

func make_zero(ind: dynamic, chce: dynamic) -> dynamic
{
  if ((chce == 1))
  {
    return;
  }
  make_zero(ind, (chce / 2));
  add(ind, ind, ind);
  if ((chce % 2))
  {
    add(ind, 5000, ind);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  read(d, p);
  curr = d;
  gdzie[0] = cpp_update(curr, "++");
  make_zero(gdzie[0], p);
  {
    var i: dynamic = 1;
    while ((i < (1 << d)))
    {
      gdzie[i] = cpp_update(curr, "++");
      var bit: dynamic = (31 - builtin_clz((i & (-i))));
      add(gdzie[(i - ((i & (-i))))], (bit + 1), gdzie[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (1 << d)))
    {
      raise(gdzie[i], gdzie[i]);
      i += 1;
    }
  }
  var sum_np: dynamic = cpp_update(curr, "++");
  add(gdzie[0], gdzie[0], sum_np);
  var ans: dynamic = cpp_update(curr, "++");
  add(gdzie[0], gdzie[0], ans);
  {
    var i: dynamic = 0;
    while ((i < (1 << d)))
    {
      if (((builtin_popcount(i) % 2) != (d % 2)))
      {
        add(sum_np, gdzie[i], sum_np);
      } else
      {
        add(ans, gdzie[i], ans);
      }
      i += 1;
    }
  }
  mult(sum_np, (p - 1));
  add(ans, sum_np, ans);
  {
    var i: dynamic = 2;
    while ((i <= d))
    {
      mult(ans, fpow(i, (p - 2)));
      i += 1;
    }
  }
  write("f ", ans, cpp_char("\n"));
  {
    var i: dynamic = 1;
    while ((i <= curr))
    {
      write(i, " ", t[i], cpp_char("\n"));
      i += 1;
    }
  }
  write(t[ans], cpp_char("\n"));
}
