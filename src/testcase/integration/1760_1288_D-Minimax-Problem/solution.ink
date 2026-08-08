// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a1: dynamic;

var a2: dynamic;

var a: dynamic;

func can(mid: dynamic)
{
  var msk = cpp_construct((1 << m), -1);
  {
    var i = 0;
    while ((i < n))
    {
      var cur = 0;
      {
        var j = 0;
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
    var i = 0;
    while ((i < ((1 << m))))
    {
      {
        var j = 0;
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

func main()
{
  read(n, m);
  a.resize(n, vector(m));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var lf = 0;
  var rg = (int_cpp(1e9) + 43);
  while (((rg - lf) > 1))
  {
    var m = (((lf + rg)) / 2);
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
