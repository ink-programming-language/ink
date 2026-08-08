// Translated from solution.cpp.

var st: dynamic;

var a: dynamic;

func get_num()
{
  var num = 0;
  while (((a != st.size()) && isdigit(st[a])))
  {
    num *= 10;
    num += (st[a] - cpp_char("0"));
    a += 1;
  }
  return num;
}

func uni_group()
{
  if ((st[a] == cpp_char("(")))
  {
    a += 1;
    var mp = mole();
    assert((st[a] == cpp_char(")")));
    a += 1;
    return mp;
  } else
  {
    var name = string_cpp(1, st[a]);
    a += 1;
    if ((((a != st.size()) && (cpp_char("a") <= st[a])) && (st[a] <= cpp_char("z"))))
    {
      name.push_back(st[a]);
      a += 1;
    }
    var amp: dynamic;
    amp[name] = 1;
    return amp;
  }
}

func group()
{
  var num = 1;
  if (((a != st.size()) && isdigit(st[a])))
  {
    num = get_num();
  }
  for (var m in mp)
  {
    m.second *= num;
  }
  return mp;
}

func mole()
{
  while (true)
  {
    if ((((a == st.size()) || (st[a] == cpp_char("+"))) || (st[a] == cpp_char(")"))))
    {
      break;
    } else
    {
      for (var n_m in n_mp)
      {
        mp[n_m.first] += n_m.second;
      }
    }
  }
  return mp;
}

var EPS = 1e-11;

func isZero(e: dynamic)
{
  return (abs(e) < EPS);
}

class Matrix
{
  var matrix: dynamic;
  var n: dynamic;
  var m: dynamic;
}

func operator_multiply(lambda: dynamic, rhs: dynamic)
{
  {
    var i = 0;
    while ((i < rhs.m))
    {
      {
        var j = 0;
        while ((j < rhs.n))
        {
          tmp.set(i, j, (tmp.get(i, j) * lambda));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp;
}

func Matrix(matrix: dynamic)
{
  cpp_base_construct(matrix);
  m = matrix.size();
  if ((m == 0))
  {
    n = 0;
  } else
  {
    n = matrix[0].size();
  }
}

func Matrix(n: dynamic)
{
  cpp_base_construct(n);
  cpp_base_construct(n);
  matrix = VV(n, Row(n, 0));
  {
    var i = 0;
    while ((i < n))
    {
      set(i, i, 1);
      i += 1;
    }
  }
}

func Matrix(row: dynamic)
{
  cpp_base_construct(1);
  cpp_base_construct(row.size());
  cpp_base_construct(VV(1, row));
  ((*this)) = transport();
}

func Matrix(m: dynamic, n: dynamic)
{
  cpp_base_construct(m);
  cpp_base_construct(n);
  matrix = VV(m, Row(n, 0));
}

func Matrix(m: dynamic, n: dynamic, e: dynamic)
{
  cpp_base_construct(m);
  cpp_base_construct(n);
  matrix = VV(m, Row(n, e));
}

func get(i: dynamic, j: dynamic)
{
  if (((((0 <= i) && (i < m)) && (0 <= j)) && (j < n)))
  {
    return matrix[i][j];
  }
  write("get(", i, ",", j, ")is not exist.", "\n");
  throw;
}

func set(i: dynamic, j: dynamic, k: dynamic)
{
  if (((((0 <= i) && (i < m)) && (0 <= j)) && (j < n)))
  {
    (*((matrix[i].begin() + j))) = k;
    return;
  }
  write("set(", i, ",", j, ")is not exist.", "\n");
  throw;
}

func operator_add(rhs: dynamic)
{
  assert(((m == rhs.m) && (n == rhs.n)));
  var tmp = cpp_construct(m, n, 0);
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = 0;
        while ((j < n))
        {
          tmp.set(i, j, (get(i, j) + rhs.get(i, j)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp;
}

func operator_multiply(rhs: dynamic)
{
  assert((n == rhs.m));
  var tmp = cpp_construct(m, rhs.n, 0);
  var sum: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = 0;
        while ((j < rhs.n))
        {
          sum = 0;
          {
            var k = 0;
            while ((k < n))
            {
              sum += (get(i, k) * rhs.get(k, j));
              k += 1;
            }
          }
          tmp.set(i, j, sum);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp;
}

func operator_subtract(rhs: dynamic)
{
  return ((*this) + ((cpp_cast(-1) * rhs)));
}

func operator_add_assign(rhs: dynamic)
{
  return cpp_assign((*this), "=", ((*this) + rhs));
}

func operator(rhs: dynamic)
{
  return cpp_assign((*this), "=", ((*this) * rhs));
}

func operator_subtract_assign(rhs: dynamic)
{
  return cpp_assign((*this), "=", ((*this) - rhs));
}

func operator_index(x: dynamic)
{
  return matrix[x];
}

func transport()
{
  var tmp: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var row: dynamic;
      {
        var j = 0;
        while ((j < m))
        {
          row.push_back(get(j, i));
          j += 1;
        }
      }
      tmp.push_back(row);
      i += 1;
    }
  }
  return tmp;
}

func pow(x: dynamic)
{
  var tmp = cpp_construct((*this));
  {
    var i = 1;
    while ((i <= x))
    {
      if ((((x & i)) > 0))
      {
        e = (e * tmp);
      }
      tmp = (tmp * tmp);
      i <<= 1;
    }
  }
  return e;
}

func cofactor(x: dynamic, y: dynamic)
{
  var tmp: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      if ((x == i))
      {
        i += 1;
        continue;
      }
      var row: dynamic;
      {
        var j = 0;
        while ((j < n))
        {
          if ((y == j))
          {
            j += 1;
            continue;
          }
          row.push_back(get(i, j));
          j += 1;
        }
      }
      tmp.push_back(row);
      i += 1;
    }
  }
  return Matrix(tmp);
}

func det()
{
  assert((n == m));
  var tri = triangulate();
  var ans = 1;
  {
    var i = 0;
    while ((i < n))
    {
      ans *= tri[i][i];
      i += 1;
    }
  }
  return ans;
  if ((m == 1))
  {
    return get(0, 0);
  }
  var sum = 0;
  {
    var i = 0;
    while ((i < m))
    {
      sum += ((((if (((i % 2) == 0)) 1 else -1) * get(i, 0))) * Matrix(cofactor(i, 0)).det());
      i += 1;
    }
  }
  return sum;
}

func triangulate()
{
  var tmp = cpp_construct((*this));
  var e: dynamic;
  var p = 0;
  {
    var i = 0;
    while (((i < m) && (p < n)))
    {
      if (isZero(tmp.get(i, p)))
      {
        tmp.set(i, p, 0);
        var flag = true;
        {
          var j = (i + 1);
          while ((j < m))
          {
            if ((!isZero(tmp.get(j, p))))
            {
              {
                var k = 0;
                while ((k < n))
                {
                  tmp.set(i, k, (tmp.get(i, k) + tmp.get(j, k)));
                  k += 1;
                }
              }
              flag = false;
              break;
            }
            j += 1;
          }
        }
        if (flag)
        {
          i -= 1;
          i += 1;
          p += 1;
          continue;
        }
      }
      {
        var j = (i + 1);
        while ((j < m))
        {
          e = (tmp.get(j, p) / tmp.get(i, p));
          {
            var k = 0;
            while ((k < n))
            {
              tmp.set(j, k, (tmp.get(j, k) - (tmp.get(i, k) * e)));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
      p += 1;
    }
  }
  return tmp;
}

func rank()
{
  {
    var i = min((tmp.m - 1), (tmp.n - 1));
    while ((i >= 0))
    {
      {
        var j = (tmp.n - 1);
        while ((j >= i))
        {
          if (isZero(tmp.get(i, j)))
          {
            j -= 1;
            continue;
          } else
          {
            return (i + 1);
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  return 0;
}

func pre_inverse()
{
  assert((m == n));
  var tmp = cpp_construct(m, n, 0);
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = 0;
        while ((j < n))
        {
          tmp.set(i, j, ((if (((((i + j)) % 2) == 0)) 1 else -1) * cofactor(i, j).det()));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp.transport();
}

func inverse()
{
  assert((m == n));
  var tmp = cpp_construct(m, (n * 2));
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = 0;
        while ((j < n))
        {
          tmp.set(i, j, get(i, j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      tmp.set(i, (i + n), 1);
      i += 1;
    }
  }
  tmp = tmp.rowReduction();
  {
    var i = 0;
    while ((i < m))
    {
      assert(isZero((tmp.get(i, i) - 1)));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = 0;
        while ((j < n))
        {
          tmp2.set(i, j, tmp.get(i, (j + n)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp2;
}

func rowReduction()
{
  var tmp = cpp_construct((*this));
  var e: dynamic;
  var p = 0;
  {
    var i = 0;
    while (((i < m) && (p < n)))
    {
      if (isZero(tmp.get(i, p)))
      {
        tmp.set(i, p, 0);
        var flag = true;
        {
          var j = (i + 1);
          while ((j < m))
          {
            if ((!isZero(tmp.get(j, p))))
            {
              {
                var k = 0;
                while ((k < n))
                {
                  tmp.set(i, k, (tmp.get(i, k) + tmp.get(j, k)));
                  k += 1;
                }
              }
              flag = false;
              break;
            }
            j += 1;
          }
        }
        if (flag)
        {
          i -= 1;
          i += 1;
          p += 1;
          continue;
        }
      }
      e = (1 / tmp.get(i, p));
      tmp.set(i, p, 1);
      {
        var k = (i + 1);
        while ((k < n))
        {
          tmp.set(i, k, (tmp.get(i, k) * e));
          k += 1;
        }
      }
      {
        var j = 0;
        while ((j < m))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          e = tmp.get(j, p);
          {
            var k = 0;
            while ((k < n))
            {
              tmp.set(j, k, (tmp.get(j, k) - (tmp.get(i, k) * e)));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
      p += 1;
    }
  }
  return tmp;
}

func mole_seq()
{
  var v: dynamic;
  v.push_back(mp);
  while (true)
  {
    if (((a == st.size()) || (st[a] == cpp_char(")"))))
    {
      break;
    } else
    {
      assert(((st[a] == cpp_char("+")) || (st[a] == cpp_char(")"))));
      a += 1;
      v.push_back(n_mp);
    }
  }
  return v;
}

func get_mat(l: dynamic, r: dynamic)
{
  var sts: dynamic;
  for (var m in l)
  {
    for (var k in m)
    {
      sts.push_back(k.first);
    }
  }
  for (var m in r)
  {
    for (var k in m)
    {
      sts.push_back(k.first);
    }
  }
  sort(sts.begin(), sts.end());
  sts.erase(unique(sts.begin(), sts.end()), sts.end());
  var mat = cpp_construct((sts.size() + 1), (((l.size() + r.size()) + 1)));
  {
    var i = 0;
    while ((i < l.size()))
    {
      for (var k in l[i])
      {
        var x = (find(sts.begin(), sts.end(), k.first) - sts.begin());
        mat[x][i] = k.second;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < r.size()))
    {
      for (var k in r[i])
      {
        var x = (find(sts.begin(), sts.end(), k.first) - sts.begin());
        mat[x][(l.size() + i)] = (-k.second);
      }
      i += 1;
    }
  }
  {
    mat[sts.size()][0] = 1;
    mat[sts.size()][(l.size() + r.size())] = 1;
  }
  return mat;
}

func main()
{
  var name: dynamic;
  while (cpp_comma((cin >> name), (name != ".")))
  {
    a = 0;
    st = name;
    var l: dynamic;
    var r: dynamic;
    var l_st = st.substr(0, st.find("->"));
    var r_st = st.substr((st.find("->") + 2));
    r_st.pop_back();
    a = 0;
    st = l_st;
    l = mole_seq();
    a = 0;
    st = r_st;
    r = mole_seq();
    var mat = get_mat(l, r);
    var ans = mat.rowReduction();
    mat = ans;
    var anss = cpp_construct((l.size() + r.size()));
    var k = 1;
    {
      k = 1;
      while ((k < 1000000))
      {
        var ok = true;
        {
          var i = 0;
          while ((i < (l.size() + r.size())))
          {
            var num = (k * mat[i][(mat[i].size() - 1)]);
            if ((abs((num - round(num))) > 1e-5))
            {
              ok = false;
            }
            i += 1;
          }
        }
        if (ok)
        {
          break;
        }
        k += 1;
      }
    }
    {
      var i = 0;
      while ((i < (l.size() + r.size())))
      {
        anss[i] = (((k * mat[i][(mat[i].size() - 1)]) + 1e-5));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < anss.size()))
      {
        write(anss[i]);
        if ((i == (anss.size() - 1)))
        {
          write("\n");
        } else
        {
          write(" ");
        }
        i += 1;
      }
    }
  }
  return 0;
}
