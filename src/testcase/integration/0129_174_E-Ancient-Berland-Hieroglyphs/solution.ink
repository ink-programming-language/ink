// Translated from solution.cpp.

var lenA: dynamic;

var lenB: dynamic;

var A = cpp_array(1000004);

var B = cpp_array(1000004);

var idxB = cpp_array(1000004);

func main()
{
  scanf("%d %d", (&lenA), (&lenB));
  {
    var i = 0;
    var n = (lenA);
    while ((i < n))
    {
      scanf("%d", (A + i));
      i += 1;
    }
  }
  memset(idxB, -1, cpp_sizeof((idxB)));
  {
    var j = 0;
    var n = (lenB);
    while ((j < n))
    {
      scanf("%d", (B + j));
      idxB[B[j]] = j;
      j += 1;
    }
  }
  var idxV = 0;
  var V: dynamic;
  var offset = 0;
  var res = 0;
  {
    var k = 0;
    var n = ((lenA * 2));
    while ((k < n))
    {
      var p = idxB[A[(k % lenA)]];
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
      var len = (int_cpp((V).size()) - idxV);
      res = max(res, len);
      k += 1;
    }
  }
  printf("%d\n", res);
  return 0;
}
