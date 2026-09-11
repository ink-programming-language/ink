// Translated from solution.cpp.

var LLINF: dynamic = (1 << 60);

class SegmentTree
{
  var id: dynamic = cpp_uninitialized();
  var op: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  var size: dynamic = cpp_uninitialized();
  func SegmentTree(n: dynamic, id: dynamic, op: dynamic) -> dynamic
  {
      self->id = cpp_construct(id);
      self->op = cpp_construct(op);
      size = 1;
      while ((size < n))
      {
        size <<= 1;
      }
      dat.assign(((size * 2) + 10), id);
    }
  func update(k: dynamic, x: dynamic) -> dynamic
  {
      k += size;
      dat[k] = x;
      while ((k > 1))
      {
        k >>= 1;
        dat[k] = op(dat[(k << 1)], dat[(((k << 1)) | 1)]);
      }
    }
  func merge(k: dynamic, x: dynamic) -> dynamic
  {
      update(k, op(x, dat[(k + size)]));
    }
  func query(a: dynamic, b: dynamic) -> dynamic
  {
      var tl: dynamic = id;
      var tr: dynamic = id;
      {
        var l: dynamic = (a + size);
        var r: dynamic = (b + size);
        while ((l < r))
        {
          if ((l & 1))
          {
            tl = op(tl, dat[cpp_update(l, "++")]);
          }
          if ((r & 1))
          {
            tr = op(tr, dat[cpp_update(r, "--")]);
          }
          l >>= 1;
          r >>= 1;
        }
      }
      return (op(tl, tr));
    }
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var seg: dynamic = cpp_construct(N, (-LLINF), __cpp_lambda_1);
  for (var v: dynamic in A)
  {
    read(v);
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      seg.update(i, 0);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var dat: dynamic = seg.query(0, A[i]);
      seg.update((A[i] - 1), (dat + A[i]));
      i += 1;
    }
  }
  write((((cpp_cast(N) * ((N + 1))) / 2) - seg.query(0, N)), "\n");
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (max(a, b));
}
