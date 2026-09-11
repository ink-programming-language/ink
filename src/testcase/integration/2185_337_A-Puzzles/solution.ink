// Translated from solution.cpp.

func main() -> dynamic
{
  var student: dynamic = cpp_uninitialized();
  var puzzles: dynamic = cpp_uninitialized();
  read(student, puzzles);
  var arr: dynamic = cpp_array(puzzles);
  var answer: dynamic = cpp_array(puzzles);
  var i: dynamic = puzzles;
  while (cpp_update(i, "--"))
  {
    read(arr[i]);
  }
  {
    var i: dynamic = 0;
    while ((i < puzzles))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < puzzles))
        {
          if ((arr[i] > arr[j]))
          {
            var temp: dynamic = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var smallest: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= (puzzles - student)))
    {
      if ((i == 0))
      {
        smallest = (arr[((student + i) - 1)] - arr[i]);
      } else if (((arr[((student + i) - 1)] - arr[i]) < smallest))
      {
        smallest = (arr[((student + i) - 1)] - arr[i]);
      }
      i += 1;
    }
  }
  write(smallest);
  return 0;
}
