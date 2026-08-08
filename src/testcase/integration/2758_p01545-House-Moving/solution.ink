// Translated from solution.cpp.

var LLINF = (1 << 60);

class SegmentTree
{
  var id: dynamic;
  var op: dynamic;
  var dat: dynamic;
  var size: dynamic;
  func SegmentTree(n: dynamic, id: dynamic, op: dynamic)
  {
      this->id = cpp_construct(id);
      this->op = cpp_construct(op);
      size = 1;
      while ((size < n))
      {
        size <<= 1;
      }
      dat.assign(((size * 2) + 10), id);
    }
  func update(k: dynamic, x: dynamic)
  {
      k += size;
      dat[k] = x;
      while ((k > 1))
      {
        k >>= 1;
        dat[k] = op(dat[(k << 1)], dat[(((k << 1)) | 1)]);
      }
    }
  func merge(k: dynamic, x: dynamic)
  {
      update(k, op(x, dat[(k + size)]));
    }
  func query(a: dynamic, b: dynamic)
  {
      var tl = id;
      var tr = id;
      {
        var l = (a + size);
        var r = (b + size);
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

func main()
{
  var N: dynamic;
  read(N);
  var seg = cpp_construct(N, (-LLINF), __cpp_lambda_1);
  for (var v in A)
  {
    read(v);
  }
  {
    var i = 0;
    while ((i < N))
    {
      seg.update(i, 0);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      var dat = seg.query(0, A[i]);
      seg.update((A[i] - 1), (dat + A[i]));
      i += 1;
    }
  }
  write((((cpp_cast(N) * ((N + 1))) / 2) - seg.query(0, N)), "\n");
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (max(a, b));
}
