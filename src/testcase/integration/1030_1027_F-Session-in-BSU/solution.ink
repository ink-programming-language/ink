// Translated from solution.cpp.

var INF: dynamic = (1 << 30);

var MAX: dynamic = (1e9 + 7);

func array_show(array: dynamic, array_n: dynamic, middle: dynamic = cpp_char(" ")) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < array_n))
    {
      printf("%d%c", array[i], ( ((i != (array_n - 1))) ? middle : cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(array: dynamic, array_n: dynamic, middle: dynamic = cpp_char(" ")) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < array_n))
    {
      printf("%lld%c", array[i], ( ((i != (array_n - 1))) ? middle : cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(vec_s: dynamic, vec_n: dynamic = -1, middle: dynamic = cpp_char(" ")) -> dynamic
{
  if ((vec_n == -1))
  {
    vec_n = vec_s.size();
  }
  {
    var i: dynamic = 0;
    while ((i < vec_n))
    {
      printf("%d%c", vec_s[i], ( ((i != (vec_n - 1))) ? middle : cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(vec_s: dynamic, vec_n: dynamic = -1, middle: dynamic = cpp_char(" ")) -> dynamic
{
  if ((vec_n == -1))
  {
    vec_n = vec_s.size();
  }
  {
    var i: dynamic = 0;
    while ((i < vec_n))
    {
      printf("%lld%c", vec_s[i], ( ((i != (vec_n - 1))) ? middle : cpp_char("\n")));
      i += 1;
    }
  }
}

class union_find_tree
{
  var uft_N: dynamic = cpp_uninitialized();
  var uft_n: dynamic = cpp_uninitialized();
  var uft_q1: dynamic = cpp_uninitialized();
  var uft_parent: dynamic = cpp_uninitialized();
  var uft_num: dynamic = cpp_uninitialized();
  var vs: dynamic = cpp_uninitialized();
  func pmax(pa: dynamic, pb: dynamic) -> dynamic
  {
      if ((pa.first < pb.first))
      {
        swap(pa, pb);
      }
      pa.second = max(pa.second, pb.first);
      return pa;
    }
  func init() -> dynamic
  {
      uft_parent.assign(uft_n, -1);
      uft_num.assign(uft_n, 1);
      vs.assign(uft_n, make_pair(-1, -1));
      {
        var i: dynamic = 0;
        while ((i < uft_n))
        {
          vs[i].first = i;
          i += 1;
        }
      }
    }
  func union_find_tree(uft_n_init: dynamic) -> dynamic
  {
      assert((uft_n_init >= 0));
      uft_n = uft_n_init;
      init();
    }
  func union_find_tree() -> dynamic
  {
      uft_n = uft_N;
      init();
    }
  func check_parent(uft_x: dynamic) -> dynamic
  {
      assert(((uft_x >= 0) && (uft_x < uft_n)));
      if ((uft_parent[uft_x] != -1))
      {
        uft_q1.push(uft_x);
        return check_parent(uft_parent[uft_x]);
      }
      var uft_a: dynamic = cpp_uninitialized();
      while ((!uft_q1.empty()))
      {
        uft_a = uft_q1.front();
        uft_q1.pop();
        uft_parent[uft_a] = uft_x;
      }
      return uft_x;
    }
  func check_max(x: dynamic) -> dynamic
  {
      x = check_parent(x);
      return vs[x].second;
    }
  func connect(uft_x: dynamic, uft_y: dynamic) -> dynamic
  {
      assert(((uft_x >= 0) && (uft_x < uft_n)));
      assert(((uft_y >= 0) && (uft_y < uft_n)));
      uft_x = check_parent(uft_x);
      uft_y = check_parent(uft_y);
      if ((uft_x == uft_y))
      {
        swap(vs[uft_x].first, vs[uft_x].second);
        vs[uft_x].first = INF;
        return true;
      }
      if ((uft_num[uft_x] > uft_num[uft_y]))
      {
        swap(uft_x, uft_y);
      }
      uft_parent[uft_x] = uft_y;
      uft_num[uft_y] += uft_num[uft_x];
      vs[uft_y] = pmax(vs[uft_y], vs[uft_x]);
      return false;
    }
  func size(pos: dynamic) -> dynamic
  {
      pos = check_parent(pos);
      return uft_num[pos];
    }
}

var m1: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var v1: dynamic = cpp_uninitialized();
  var va: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&a), (&b));
      v1.push_back(make_pair(a, b));
      m1[a] = 0;
      m1[b] = 0;
      i += 1;
    }
  }
  i = 0;
  for (var node: dynamic in m1)
  {
    node.second = cpp_update(i, "++");
    va.push_back(node.first);
  }
  m = va.size();
  {
    i = 0;
    while ((i < n))
    {
      ua.connect(m1[v1[i].first], m1[v1[i].second]);
      i += 1;
    }
  }
  a = 0;
  {
    i = 0;
    while ((i < m))
    {
      a = max(a, ua.check_max(i));
      i += 1;
    }
  }
  if ((a >= INF))
  {
    write(-1, "\n");
  } else
  {
    write(va[a], "\n");
  }
}
