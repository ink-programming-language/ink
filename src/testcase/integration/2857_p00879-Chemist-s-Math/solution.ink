// Translated from solution.cpp.

var st: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

func get_num() -> dynamic
{
  var num: dynamic = 0;
  while (((a != st.size()) && isdigit(st[a])))
  {
    num *= 10;
    num += (st[a] - cpp_char("0"));
    a += 1;
  }
  return num;
}

func uni_group() -> dynamic
{
  if ((st[a] == cpp_char("(")))
  {
    a += 1;
    var mp: dynamic = mole();
    assert((st[a] == cpp_char(")")));
    a += 1;
    return mp;
  } else
  {
    var name: dynamic = string_cpp(1, st[a]);
    a += 1;
    if ((((a != st.size()) && (cpp_char("a") <= st[a])) && (st[a] <= cpp_char("z"))))
    {
      name.push_back(st[a]);
      a += 1;
    }
    var amp: dynamic = cpp_uninitialized();
    amp[name] = 1;
    return amp;
  }
}

func group() -> dynamic
{
  var num: dynamic = 1;
  if (((a != st.size()) && isdigit(st[a])))
  {
    num = get_num();
  }
  for (var m: dynamic in mp)
  {
    m.second *= num;
  }
  return mp;
}

func mole() -> dynamic
{
  while (true)
  {
    if ((((a == st.size()) || (st[a] == cpp_char("+"))) || (st[a] == cpp_char(")"))))
    {
      break;
    } else
    {
      for (var n_m: dynamic in n_mp)
      {
        mp[n_m.first] += n_m.second;
      }
    }
  }
  return mp;
}

var EPS: dynamic = 1e-11;

func isZero(e: dynamic) -> dynamic
{
  return (abs(e) < EPS);
}

class Matrix
{
  var matrix: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
}

func operator_multiply(lambda: dynamic, rhs: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < rhs.m))
    {
      {
        var j: dynamic = 0;
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

func Matrix(matrix: dynamic) -> dynamic
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

func Matrix(n: dynamic) -> dynamic
{
  cpp_base_construct(n);
  cpp_base_construct(n);
  matrix = VV(n, Row(n, 0));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      set(i, i, 1);
      i += 1;
    }
  }
}

func Matrix(row: dynamic) -> dynamic
{
  cpp_base_construct(1);
  cpp_base_construct(row.size());
  cpp_base_construct(VV(1, row));
  ((*self)) = transport();
}

func Matrix(m: dynamic, n: dynamic) -> dynamic
{
  cpp_base_construct(m);
  cpp_base_construct(n);
  matrix = VV(m, Row(n, 0));
}

func Matrix(m: dynamic, n: dynamic, e: dynamic) -> dynamic
{
  cpp_base_construct(m);
  cpp_base_construct(n);
  matrix = VV(m, Row(n, e));
}

func get(i: dynamic, j: dynamic) -> dynamic
{
  if (((((0 <= i) && (i < m)) && (0 <= j)) && (j < n)))
  {
    return matrix[i][j];
  }
  write("get(", i, ",", j, ")is not exist.", "\n");
  return 0;
}

func set(i: dynamic, j: dynamic, k: dynamic) -> dynamic
{
  if (((((0 <= i) && (i < m)) && (0 <= j)) && (j < n)))
  {
    (*((matrix[i].begin() + j))) = k;
    return;
  }
  write("set(", i, ",", j, ")is not exist.", "\n");
  return;
}

func operator_add(rhs: dynamic) -> dynamic
{
  assert(((m == rhs.m) && (n == rhs.n)));
  var tmp: dynamic = cpp_construct(m, n, 0);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = 0;
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

func operator_multiply(rhs: dynamic) -> dynamic
{
  assert((n == rhs.m));
  var tmp: dynamic = cpp_construct(m, rhs.n, 0);
  var sum: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = 0;
        while ((j < rhs.n))
        {
          sum = 0;
          {
            var k: dynamic = 0;
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

func operator_subtract(rhs: dynamic) -> dynamic
{
  return ((*self) + ((cpp_cast(-1) * rhs)));
}

func operator_add_assign(rhs: dynamic) -> dynamic
{
  return cpp_assign((*self), "=", ((*self) + rhs));
}

func operator(rhs: dynamic) -> dynamic
{
  return cpp_assign((*self), "=", ((*self) * rhs));
}

func operator_subtract_assign(rhs: dynamic) -> dynamic
{
  return cpp_assign((*self), "=", ((*self) - rhs));
}

func operator_index(x: dynamic) -> dynamic
{
  return matrix[x];
}

func transport() -> dynamic
{
  var tmp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var row: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
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

func pow(x: dynamic) -> dynamic
{
  var tmp: dynamic = cpp_construct((*self));
  {
    var i: dynamic = 1;
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

func cofactor(x: dynamic, y: dynamic) -> dynamic
{
  var tmp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if ((x == i))
      {
        i += 1;
        continue;
      }
      var row: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
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

func det() -> dynamic
{
  assert((n == m));
  var tri: dynamic = triangulate();
  var ans: dynamic = 1;
  {
    var i: dynamic = 0;
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
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      sum += (((( (((i % 2) == 0)) ? 1 : -1) * get(i, 0))) * Matrix(cofactor(i, 0)).det());
      i += 1;
    }
  }
  return sum;
}

func triangulate() -> dynamic
{
  var tmp: dynamic = cpp_construct((*self));
  var e: dynamic = cpp_uninitialized();
  var p: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < m) && (p < n)))
    {
      if (isZero(tmp.get(i, p)))
      {
        tmp.set(i, p, 0);
        var flag: dynamic = true;
        {
          var j: dynamic = (i + 1);
          while ((j < m))
          {
            if ((!isZero(tmp.get(j, p))))
            {
              {
                var k: dynamic = 0;
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
        var j: dynamic = (i + 1);
        while ((j < m))
        {
          e = (tmp.get(j, p) / tmp.get(i, p));
          {
            var k: dynamic = 0;
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

func rank() -> dynamic
{
  {
    var i: dynamic = min((tmp.m - 1), (tmp.n - 1));
    while ((i >= 0))
    {
      {
        var j: dynamic = (tmp.n - 1);
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

func pre_inverse() -> dynamic
{
  assert((m == n));
  var tmp: dynamic = cpp_construct(m, n, 0);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          tmp.set(i, j, (( (((((i + j)) % 2) == 0)) ? 1 : -1) * cofactor(i, j).det()));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return tmp.transport();
}

func inverse() -> dynamic
{
  assert((m == n));
  var tmp: dynamic = cpp_construct(m, (n * 2));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < m))
    {
      tmp.set(i, (i + n), 1);
      i += 1;
    }
  }
  tmp = tmp.rowReduction();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      assert(isZero((tmp.get(i, i) - 1)));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = 0;
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

func rowReduction() -> dynamic
{
  var tmp: dynamic = cpp_construct((*self));
  var e: dynamic = cpp_uninitialized();
  var p: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < m) && (p < n)))
    {
      if (isZero(tmp.get(i, p)))
      {
        tmp.set(i, p, 0);
        var flag: dynamic = true;
        {
          var j: dynamic = (i + 1);
          while ((j < m))
          {
            if ((!isZero(tmp.get(j, p))))
            {
              {
                var k: dynamic = 0;
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
        var k: dynamic = (i + 1);
        while ((k < n))
        {
          tmp.set(i, k, (tmp.get(i, k) * e));
          k += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          e = tmp.get(j, p);
          {
            var k: dynamic = 0;
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

func mole_seq() -> dynamic
{
  var v: dynamic = cpp_uninitialized();
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

func get_mat(l: dynamic, r: dynamic) -> dynamic
{
  var sts: dynamic = cpp_uninitialized();
  for (var m: dynamic in l)
  {
    for (var k: dynamic in m)
    {
      sts.push_back(k.first);
    }
  }
  for (var m: dynamic in r)
  {
    for (var k: dynamic in m)
    {
      sts.push_back(k.first);
    }
  }
  sort(sts.begin(), sts.end());
  sts.erase(unique(sts.begin(), sts.end()), sts.end());
  var mat: dynamic = cpp_construct((sts.size() + 1), (((l.size() + r.size()) + 1)));
  {
    var i: dynamic = 0;
    while ((i < l.size()))
    {
      for (var k: dynamic in l[i])
      {
        var x: dynamic = (find(sts.begin(), sts.end(), k.first) - sts.begin());
        mat[x][i] = k.second;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < r.size()))
    {
      for (var k: dynamic in r[i])
      {
        var x: dynamic = (find(sts.begin(), sts.end(), k.first) - sts.begin());
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

func main() -> dynamic
{
  var name: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> name), (name != ".")))
  {
    a = 0;
    st = name;
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var l_st: dynamic = st.substr(0, st.find("->"));
    var r_st: dynamic = st.substr((st.find("->") + 2));
    r_st.pop_back();
    a = 0;
    st = l_st;
    l = mole_seq();
    a = 0;
    st = r_st;
    r = mole_seq();
    var mat: dynamic = get_mat(l, r);
    var ans: dynamic = mat.rowReduction();
    mat = ans;
    var anss: dynamic = cpp_construct((l.size() + r.size()));
    var k: dynamic = 1;
    {
      k = 1;
      while ((k < 1000000))
      {
        var ok: dynamic = true;
        {
          var i: dynamic = 0;
          while ((i < (l.size() + r.size())))
          {
            var num: dynamic = (k * mat[i][(mat[i].size() - 1)]);
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
      var i: dynamic = 0;
      while ((i < (l.size() + r.size())))
      {
        anss[i] = (((k * mat[i][(mat[i].size() - 1)]) + 1e-5));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
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
