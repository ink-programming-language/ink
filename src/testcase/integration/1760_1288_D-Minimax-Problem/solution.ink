// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a1: dynamic = cpp_uninitialized();

var a2: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

func can(mid: dynamic) -> dynamic
{
  var msk: dynamic = cpp_construct((1 << m), -1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var cur: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((a[i][j] >= mid))
          {
            cur ^= ((1 << j));
          }
          j += 1;
        }
      }
      msk[cur] = i;
      i += 1;
    }
  }
  if ((msk[(((1 << m)) - 1)] != -1))
  {
    a1 = cpp_assign(a2, "=", msk[(((1 << m)) - 1)]);
    return true;
  }
  {
    var i: dynamic = 0;
    while ((i < ((1 << m))))
    {
      {
        var j: dynamic = 0;
        while ((j < ((1 << m))))
        {
          if ((((msk[i] != -1) && (msk[j] != -1)) && (((i | j)) == (((1 << m)) - 1))))
          {
            a1 = msk[i];
            a2 = msk[j];
            return true;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return false;
}

func main() -> dynamic
{
  read(n, m);
  a.resize(n, vector(m));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var lf: dynamic = 0;
  var rg: dynamic = (int_cpp(1e9) + 43);
  while (((rg - lf) > 1))
  {
    var m: dynamic = (((lf + rg)) / 2);
    if (can(m))
    {
      lf = m;
    } else
    {
      rg = m;
    }
  }
  write((a1 + 1), " ", (a2 + 1));
}
