// Translated from solution.cpp.

var A = cpp_array(5001);

var B = cpp_array(5001);

var C = cpp_array(5001);

var D = cpp_array(5001);

var N: dynamic;

var M: dynamic;

var S = [];

func main()
{
  read(N);
  {
    var i = 0;
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
  var T = 0;
  {
    var i = 1;
    while ((i < (N + 1)))
    {
      T = 1;
      {
        var j = 1;
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
