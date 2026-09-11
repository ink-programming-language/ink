// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < N))
    {
      read(m);
      {
        j = 0;
        while ((j < m))
        {
          read(c);
          C[i].push_back(c);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  {
    i = 0;
    while ((i < N))
    {
      {
        j = 0;
        while ((j < C[((N - i) - 1)].size()))
        {
          q.push(C[((N - i) - 1)][j]);
          j += 1;
        }
      }
      ans += q.top();
      q.pop();
      i += 1;
    }
  }
  write(ans, "\n");
}
