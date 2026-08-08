// Translated from solution.cpp.

func main()
{
  var student: dynamic;
  var puzzles: dynamic;
  read(student, puzzles);
  var arr = cpp_array(puzzles);
  var answer = cpp_array(puzzles);
  var i = puzzles;
  while (cpp_update(i, "--"))
  {
    read(arr[i]);
  }
  {
    var i = 0;
    while ((i < puzzles))
    {
      {
        var j = (i + 1);
        while ((j < puzzles))
        {
          if ((arr[i] > arr[j]))
          {
            var temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var smallest = 0;
  {
    var i = 0;
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
