// Translated from solution.cpp.

var MAX = 5010;

var N: dynamic;

var K: dynamic;

var A = cpp_array(MAX);

var B = cpp_array(MAX);

var D = cpp_array(MAX, MAX);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(N, K);
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  var j = 0;
  sort(A, (A + N));
  {
    var i = 0;
    while ((i < N))
    {
      while ((((j + 1) < N) && (A[(j + 1)] <= (A[i] + 5))))
      {
        j += 1;
      }
      B[i] = j;
      i += 1;
    }
  }
  {
    var j = 1;
    while ((j <= K))
    {
      {
        var i = (N - 1);
        while ((i >= 0))
        {
          D[i][j] = max(D[(i + 1)][j], (((D[(B[i] + 1)][(j - 1)] + B[i]) - i) + 1));
          i -= 1;
        }
      }
      j += 1;
    }
  }
  write(D[0][K]);
}
