// Translated from solution.cpp.

var INF = (1 << 30);

var MAX = (1e9 + 7);

func array_show(array: dynamic, array_n: dynamic, middle: dynamic = cpp_char(" "))
{
  {
    var i = 0;
    while ((i < array_n))
    {
      printf("%d%c", array[i], (if ((i != (array_n - 1))) middle else cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(array: dynamic, array_n: dynamic, middle: dynamic = cpp_char(" "))
{
  {
    var i = 0;
    while ((i < array_n))
    {
      printf("%lld%c", array[i], (if ((i != (array_n - 1))) middle else cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(vec_s: dynamic, vec_n: dynamic = -1, middle: dynamic = cpp_char(" "))
{
  if ((vec_n == -1))
  {
    vec_n = vec_s.size();
  }
  {
    var i = 0;
    while ((i < vec_n))
    {
      printf("%d%c", vec_s[i], (if ((i != (vec_n - 1))) middle else cpp_char("\n")));
      i += 1;
    }
  }
}

func array_show(vec_s: dynamic, vec_n: dynamic = -1, middle: dynamic = cpp_char(" "))
{
  if ((vec_n == -1))
  {
    vec_n = vec_s.size();
  }
  {
    var i = 0;
    while ((i < vec_n))
    {
      printf("%lld%c", vec_s[i], (if ((i != (vec_n - 1))) middle else cpp_char("\n")));
      i += 1;
    }
  }
}

class union_find_tree
{
  var uft_N: dynamic;
  var uft_n: dynamic;
  var uft_q1: dynamic;
  var uft_parent: dynamic;
  var uft_num: dynamic;
  var vs: dynamic;
  func pmax(pa: dynamic, pb: dynamic)
  {
      if ((pa.first < pb.first))
      {
        swap(pa, pb);
      }
      pa.second = max(pa.second, pb.first);
      return pa;
    }
  func init()
  {
      uft_parent.assign(uft_n, -1);
      uft_num.assign(uft_n, 1);
      vs.assign(uft_n, make_pair(-1, -1));
      {
        var i = 0;
        while ((i < uft_n))
        {
          vs[i].first = i;
          i += 1;
        }
      }
    }
  func union_find_tree(uft_n_init: dynamic)
  {
      assert((uft_n_init >= 0));
      uft_n = uft_n_init;
      init();
    }
  func union_find_tree()
  {
      uft_n = uft_N;
      init();
    }
  func check_parent(uft_x: dynamic)
  {
      assert(((uft_x >= 0) && (uft_x < uft_n)));
      if ((uft_parent[uft_x] != -1))
      {
        uft_q1.push(uft_x);
        return check_parent(uft_parent[uft_x]);
      }
      var uft_a: dynamic;
      while ((!uft_q1.empty()))
      {
        uft_a = uft_q1.front();
        uft_q1.pop();
        uft_parent[uft_a] = uft_x;
      }
      return uft_x;
    }
  func check_max(x: dynamic)
  {
      x = check_parent(x);
      return vs[x].second;
    }
  func connect(uft_x: dynamic, uft_y: dynamic)
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
  func size(pos: dynamic)
  {
      pos = check_parent(pos);
      return uft_num[pos];
    }
}

var m1: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  scanf("%d", (&n));
  var v1: dynamic;
  var va: dynamic;
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
  for (var node in m1)
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
