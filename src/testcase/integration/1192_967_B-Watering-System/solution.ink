// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  read(n, A, B);
  {
    int_cpp(i) = 0;
    while (((i) < (n)))
    {
      read(v[i]);
      i += 1;
    }
  }
  var sum: dynamic = accumulate(v.begin(), v.end(), 0);
  var flow: dynamic = ((A * v[0]) / sum);
  var ans: dynamic = 0;
  sort((v.begin() + 1), v.end());
  reverse((v.begin() + 1), v.end());
  var i: dynamic = 1;
  while ((flow < B))
  {
    sum -= v[i];
    ans += 1;
    i += 1;
    flow = ((A * v[0]) / sum);
  }
  write(ans, cpp_char("\n"));
  return 0;
}
