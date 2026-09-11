// Translated from solution.cpp.

var A: dynamic = cpp_array(5001);

var B: dynamic = cpp_array(5001);

var C: dynamic = cpp_array(5001);

var D: dynamic = cpp_array(5001);

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var S: dynamic = [];

func main() -> dynamic
{
  read(N);
  {
    var i: dynamic = 0;
    while ((i < (((N * ((N - 1))) / 2))))
    {
      read(A[i], B[i], C[i], D[i]);
      if ((C[i] > D[i]))
      {
        S[A[i]] += 3;
      }
      if ((C[i] < D[i]))
      {
        S[B[i]] += 3;
      }
      if ((C[i] == D[i]))
      {
        S[A[i]] += 1;
        S[B[i]] += 1;
      }
      i += 1;
    }
  }
  var T: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < (N + 1)))
    {
      T = 1;
      {
        var j: dynamic = 1;
        while ((j < (N + 1)))
        {
          if ((S[i] < S[j]))
          {
            T += 1;
          }
          j += 1;
        }
      }
      write(T, "\n");
      i += 1;
    }
  }
  return 0;
}
