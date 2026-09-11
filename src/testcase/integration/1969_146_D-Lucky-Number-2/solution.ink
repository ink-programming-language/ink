// Translated from solution.cpp.

func max(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? a : b;
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? b : a;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var a4: dynamic = cpp_uninitialized();
  var a7: dynamic = cpp_uninitialized();
  var a47: dynamic = cpp_uninitialized();
  var a74: dynamic = cpp_uninitialized();
  scanf("%d %d %d %d", (&a4), (&a7), (&a47), (&a74));
  if ((fabs((a47 - a74)) > 1))
  {
    printf("-1\n");
    return 0;
  }
  if ((a47 != a74))
  {
    if ((min(a4, a7) < max(a47, a74)))
    {
      printf("-1\n");
      return 0;
    }
  }
  if ((a47 == a74))
  {
    if ((!(((((a4 > a47) && (a7 >= a47))) || (((a4 >= a47) && (a7 > a47)))))))
    {
      printf("-1\n");
      return 0;
    }
  }
  if ((a47 > a74))
  {
    {
      i = 1;
      while ((i <= (a4 - a47)))
      {
        printf("4");
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= a47))
      {
        printf("47");
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= (a7 - a47)))
      {
        printf("7");
        i += 1;
      }
    }
    printf("\n");
  } else if ((a47 < a74))
  {
    printf("74");
    {
      i = 1;
      while ((i <= (a4 - a74)))
      {
        printf("4");
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= (a74 - 2)))
      {
        printf("74");
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= ((a7 - a74) + 1)))
      {
        printf("7");
        i += 1;
      }
    }
    printf("4\n");
  } else if ((a47 == a74))
  {
    if ((a4 == a47))
    {
      {
        i = 1;
        while ((i <= a47))
        {
          printf("74");
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= (a7 - a47)))
        {
          printf("7");
          i += 1;
        }
      }
      printf("\n");
    } else
    {
      {
        i = 1;
        while ((i <= ((a4 - a47) - 1)))
        {
          printf("4");
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= a47))
        {
          printf("47");
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= (a7 - a47)))
        {
          printf("7");
          i += 1;
        }
      }
      printf("4\n");
    }
  }
}
