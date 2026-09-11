// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(500000);

var c: dynamic = cpp_array(500000);

var a: dynamic = cpp_array(500000);

var W: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(500000);

var t: dynamic = cpp_array(500000);

var r_fraction: dynamic = cpp_uninitialized();

var r_integer: dynamic = cpp_uninitialized();

func take(nr: dynamic, amount: dynamic) -> dynamic
{
  if ((!amount))
  {
    return;
  }
  r_integer += ((cpp_cast(amount) * c[nr]) / w[nr]);
  var p: dynamic = ((cpp_cast(amount) * c[nr]) % w[nr]);
  var q: dynamic = w[nr];
  r_fraction += (cpp_cast(p) / q);
  while ((r_fraction >= 1))
  {
    r_fraction -= 1;
    r_integer += 1;
  }
}

func select(from_cpp: dynamic, to: dynamic, left: dynamic) -> dynamic
{
  if ((from_cpp == to))
  {
    return;
  }
  if (((from_cpp + 1) == to))
  {
    take(s[from_cpp], left);
    return;
  }
  var middle_t: dynamic = t[s[(((from_cpp + to)) / 2)]];
  var middle: dynamic = from_cpp;
  {
    var i: dynamic = from_cpp;
    while ((i < to))
    {
      if ((t[s[i]] < middle_t))
      {
        swap(s[i], s[cpp_update(middle, "++")]);
      }
      i += 1;
    }
  }
  var less_sum: dynamic = 0;
  var eq_sum: dynamic = 0;
  {
    var i: dynamic = from_cpp;
    while ((i < middle))
    {
      less_sum += w[s[i]];
      i += 1;
    }
  }
  if ((less_sum >= left))
  {
    select(from_cpp, middle, left);
    return;
  }
  {
    var i: dynamic = from_cpp;
    while ((i < middle))
    {
      take(s[i], w[s[i]]);
      left -= w[s[i]];
      i += 1;
    }
  }
  var middle2: dynamic = middle;
  {
    var i: dynamic = middle;
    while ((i < to))
    {
      if ((t[s[i]] == middle_t))
      {
        swap(s[i], s[cpp_update(middle2, "++")]);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = middle;
    while ((i < middle2))
    {
      eq_sum += w[s[i]];
      i += 1;
    }
  }
  if ((eq_sum >= left))
  {
    take(s[middle], left);
    return;
  }
  take(s[middle], eq_sum);
  left -= eq_sum;
  select(middle2, to, left);
}

func main() -> dynamic
{
  scanf("%d %d %d", (&n), (&m), (&W));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&w[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      t[i] = (cpp_cast(c[i]) / w[i]);
      s[i] = i;
      i += 1;
    }
  }
  random_shuffle(s, (s + m));
  {
    var z: dynamic = 0;
    while ((z < n))
    {
      select(0, m, W);
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          t[i] -= (cpp_cast(a[i]) / w[i]);
          c[i] -= a[i];
          i += 1;
        }
      }
      z += 1;
    }
  }
  var buf: dynamic = cpp_array(20);
  sprintf(buf, "%.12lf", r_fraction);
  if ((buf[0] == cpp_char("1")))
  {
    r_integer += 1;
  }
  write(r_integer, (buf + 1), "\n");
}
