// Translated from solution.cpp.

func main() -> dynamic
{
  var arr: dynamic = cpp_array(100005);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  while ((scanf("%d %d %d", (&n), (&a), (&b)) == 3))
  {
    getchar();
    {
      i = 1;
      while ((i <= n))
      {
        scanf("%c", (&arr[i]));
        i += 1;
      }
    }
    if ((arr[a] != arr[b]))
    {
      printf("1\n");
    } else if ((arr[a] == arr[b]))
    {
      printf("0\n");
    }
  }
  return 0;
}
