// Translated from solution.cpp.

var lenA: dynamic = cpp_uninitialized();

var lenB: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(1000004);

var B: dynamic = cpp_array(1000004);

var idxB: dynamic = cpp_array(1000004);

func main() -> dynamic
{
  scanf("%d %d", (&lenA), (&lenB));
  {
    var i: dynamic = 0;
    var n: dynamic = (lenA);
    while ((i < n))
    {
      scanf("%d", (A + i));
      i += 1;
    }
  }
  memset(idxB, -1, cpp_sizeof((idxB)));
  {
    var j: dynamic = 0;
    var n: dynamic = (lenB);
    while ((j < n))
    {
      scanf("%d", (B + j));
      idxB[B[j]] = j;
      j += 1;
    }
  }
  var idxV: dynamic = 0;
  var V: dynamic = cpp_uninitialized();
  var offset: dynamic = 0;
  var res: dynamic = 0;
  {
    var k: dynamic = 0;
    var n: dynamic = ((lenA * 2));
    while ((k < n))
    {
      var p: dynamic = idxB[A[(k % lenA)]];
      if ((p < 0))
      {
        V.clear();
        idxV = 0;
        k += 1;
        continue;
      }
      p += offset;
      if (((!V.empty()) && (V.back() >= p)))
      {
        p += lenB;
        offset += lenB;
      }
      V.push_back(p);
      idxV = ((lower_bound((V.begin() + idxV), V.end(), ((p - lenB) + 1)) - V.begin()));
      var len: dynamic = (int_cpp((V).size()) - idxV);
      res = max(res, len);
      k += 1;
    }
  }
  printf("%d\n", res);
  return 0;
}
