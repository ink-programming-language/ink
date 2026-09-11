// Translated from solution.cpp.

var MAX: dynamic = 5010;

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(MAX);

var B: dynamic = cpp_array(MAX);

var D: dynamic = cpp_array(MAX, MAX);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(N, K);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  var j: dynamic = 0;
  sort(A, (A + N));
  {
    var i: dynamic = 0;
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
    var j: dynamic = 1;
    while ((j <= K))
    {
      {
        var i: dynamic = (N - 1);
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
