// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var nex: dynamic = cpp_array(2000000);

var hea: dynamic = cpp_array(2000000);

var wen: dynamic = cpp_array(2000000);

var val: dynamic = cpp_array(2000000);

var aid: dynamic = cpp_array(2000000);

var root2: dynamic = cpp_uninitialized();

var root1: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

var maxx: dynamic = cpp_uninitialized();

class segment_tree
{
  var a: dynamic = cpp_array(2, 2000000);
  var lazy: dynamic = cpp_array(2000000);
  var fa: dynamic = cpp_array(1000000);
  var dep: dynamic = cpp_array(1000000);
  var in_cpp: dynamic = cpp_array(1000000);
  var out: dynamic = cpp_array(1000000);
  var m: dynamic = cpp_uninitialized();
  func pushdown(k: dynamic) -> dynamic
  {
      if ((!lazy[k]))
      {
        return;
      }
      swap(a[(k << 1)][0], a[(k << 1)][1]);
      swap(a[(((k << 1)) | 1)][0], a[(((k << 1)) | 1)][1]);
      lazy[(k << 1)] ^= 1;
      lazy[(((k << 1)) | 1)] ^= 1;
      lazy[k] = 0;
    }
  func update1(l: dynamic, r: dynamic, k: dynamic, x: dynamic, y: dynamic, z: dynamic) -> dynamic
  {
      if ((l == r))
      {
        a[k][z] = y;
        return;
      }
      var mid: dynamic = (((l + r)) >> 1);
      if ((x <= mid))
      {
        update1(l, mid, (k << 1), x, y, z);
      }
      if ((x > mid))
      {
        update1((mid + 1), r, (((k << 1)) | 1), x, y, z);
      }
      a[k][0] = max(a[(k << 1)][0], a[(((k << 1)) | 1)][0]);
      a[k][1] = max(a[(k << 1)][1], a[(((k << 1)) | 1)][1]);
    }
  func update2(l: dynamic, r: dynamic, k: dynamic, x: dynamic, y: dynamic) -> dynamic
  {
      if (((l >= x) && (r <= y)))
      {
        lazy[k] ^= 1;
        swap(a[k][0], a[k][1]);
        return;
      }
      pushdown(k);
      var mid: dynamic = (((l + r)) >> 1);
      if ((x <= mid))
      {
        update2(l, mid, (k << 1), x, y);
      }
      if ((y > mid))
      {
        update2((mid + 1), r, (((k << 1)) | 1), x, y);
      }
      a[k][0] = max(a[(k << 1)][0], a[(((k << 1)) | 1)][0]);
      a[k][1] = max(a[(k << 1)][1], a[(((k << 1)) | 1)][1]);
    }
  func build(x: dynamic, y: dynamic, z: dynamic) -> dynamic
  {
      var a: dynamic = 0;
      in_cpp[x] = cpp_update(m, "++");
      dep[x] = (dep[y] + 1);
      {
        var i: dynamic = hea[x];
        while (i)
        {
          if ((wen[i] != y))
          {
            a += 1;
            fa[aid[i]] = wen[i];
            build(wen[i], x, (z ^ val[i]));
          }
          i = nex[i];
        }
      }
      update1(1, n, 1, in_cpp[x], dep[x], z);
      out[x] = m;
    }
  func revers(x: dynamic) -> dynamic
  {
      update2(1, n, 1, in_cpp[fa[x]], out[fa[x]]);
    }
}

var st1: dynamic = cpp_uninitialized();

var st2: dynamic = cpp_uninitialized();

func add(x: dynamic, y: dynamic, z: dynamic, p: dynamic) -> dynamic
{
  len += 1;
  nex[len] = hea[x];
  wen[len] = y;
  val[len] = z;
  aid[len] = p;
  hea[x] = len;
}

func dfs(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  if ((z >= maxx))
  {
    maxx = z;
    root1 = x;
  }
  {
    var i: dynamic = hea[x];
    while (i)
    {
      if ((wen[i] != y))
      {
        dfs(wen[i], x, (z + 1));
      }
      i = nex[i];
    }
  }
}

func dfs1(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  if ((z >= maxx))
  {
    maxx = z;
    root2 = x;
  }
  {
    var i: dynamic = hea[x];
    while (i)
    {
      if ((wen[i] != y))
      {
        dfs1(wen[i], x, (z + 1));
      }
      i = nex[i];
    }
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&x), (&y), (&z));
      add(x, y, z, i);
      add(y, x, z, i);
      i += 1;
    }
  }
  dfs(1, 0, 0);
  dfs1(root1, 0, 0);
  st1.build(root1, 0, 0);
  st2.build(root2, 0, 0);
  scanf("%d", (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      st1.revers(x);
      st2.revers(x);
      printf(" %d\n", (max(st1.a[1][0], st2.a[1][0]) - 1));
      i += 1;
    }
  }
  return 0;
}
